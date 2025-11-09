// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext(
                            "Usage: %s [OPTION]... [-T] SOURCE DEST\n  or:  %s [OPTION]... SOURCE... DIRECTORY\n  or:  %s [OPTION]... -t DIRECTORY SOURCE...\n"
                            );
    printf(pcVar3,uVar2,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("Copy SOURCE to DEST, or multiple SOURCE(s) to DIRECTORY.\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -a, --archive                same as -dR --preserve=all\n      --attributes-only        don\'t copy the file data, just the attributes\n      --backup[=CONTROL]       make a backup of each existing destination file\n  -b                           like --backup but does not accept an argument\n      --copy-contents          copy contents of special files when recursive\n  -d                           same as --no-dereference --preserve=links\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --debug                  explain how a file is copied.  Implies -v\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -f, --force                  if an existing destination file cannot be\n                                 opened, remove it and try again (this option\n                                 is ignored when the -n option is also used)\n  -i, --interactive            prompt before overwrite (overrides a previous -n\n                                  option)\n  -H                           follow command-line symbolic links in SOURCE\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -l, --link                   hard link files instead of copying\n  -L, --dereference            always follow symbolic links in SOURCE\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -n, --no-clobber             (deprecated) silently skip existing files.\n                                 See also --update\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -P, --no-dereference         never follow symbolic links in SOURCE\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -p                           same as --preserve=mode,ownership,timestamps\n      --preserve[=ATTR_LIST]   preserve the specified attributes\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --no-preserve=ATTR_LIST  don\'t preserve the specified attributes\n      --parents                use full source file name under DIRECTORY\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -R, -r, --recursive          copy directories recursively\n      --reflink[=WHEN]         control clone/CoW copies. See below\n      --remove-destination     remove each existing destination file before\n                                 attempting to open it (contrast with --force)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --sparse=WHEN            control creation of sparse files. See below\n      --strip-trailing-slashes  remove any trailing slashes from each SOURCE\n                                 argument\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -s, --symbolic-link          make symbolic links instead of copying\n  -S, --suffix=SUFFIX          override the usual backup suffix\n  -t, --target-directory=DIRECTORY  copy all SOURCE arguments into DIRECTORY\n  -T, --no-target-directory    treat DEST as a normal file\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --update[=UPDATE]        control which existing files are updated;\n                                 UPDATE={all,none,none-fail,older(default)}\n  -u                           equivalent to --update[=older].  See below\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -v, --verbose                explain what is being done\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --keep-directory-symlink  follow existing symlinks to directories\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -x, --one-file-system        stay on this file system\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -Z                           set SELinux security context of destination\n                                 file to default type\n      --context[=CTX]          like -Z, or if CTX is specified then set the\n                                 SELinux or SMACK security context to CTX\n"
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
                            "\nATTR_LIST is a comma-separated list of attributes. Attributes are \'mode\' for\npermissions (including any ACL and xattr permissions), \'ownership\' for user\nand group, \'timestamps\' for file timestamps, \'links\' for hard links, \'context\'\nfor security context, \'xattr\' for extended attributes, and \'all\' for all\nattributes.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nBy default, sparse SOURCE files are detected by a crude heuristic and the\ncorresponding DEST file is made sparse as well.  That is the behavior\nselected by --sparse=auto.  Specify --sparse=always to create a sparse DEST\nfile whenever the SOURCE file contains a long enough sequence of zero bytes.\nUse --sparse=never to inhibit creation of sparse files.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_update_parameters_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nBy default or with --reflink=auto, cp will try a lightweight copy, where the\ndata blocks are copied only when modified, falling back to a standard copy\nif this is not possible.  With --reflink[=always] cp will fail if CoW is not\nsupported, while --reflink=never ensures a standard copy is performed.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_backup_suffix_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nAs a special case, cp makes a backup of SOURCE when the force and backup\noptions are given and SOURCE and DEST are the same name for an existing,\nregular file.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_001048f2);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: re_protect
undefined8 re_protect(char *param_1,long param_2,int param_3,long param_4,long param_5,long param_6) {
  undefined4 uVar1;
  undefined4 uVar2;
  long lVar3;
  long lVar4;
  char *pcVar5;
  void *pvVar6;
  char cVar7;
  int iVar8;
  size_t sVar9;
  ulong uVar10;
  undefined8 uVar11;
  int *piVar12;
  undefined8 uVar13;
  long *plVar14;
  long in_FS_OFFSET;
  timespec tVar15;
  undefined auVar16 [16];
  long local_e8;
  long local_e0;
  long local_d8;
  int local_cc;
  long local_c8;
  char *local_c0;
  undefined4 local_b4;
  undefined4 local_b0;
  int local_ac;
  undefined4 local_a8;
  int local_a4;
  undefined4 local_a0;
  int local_9c;
  long local_98;
  char *local_90;
  size_t local_88;
  void *local_80;
  void *local_78;
  long local_70;
  char *local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  timespec local_48;
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_e8 = param_6;
  local_e0 = param_5;
  local_d8 = param_4;
  local_cc = param_3;
  local_c8 = param_2;
  local_c0 = param_1;
  local_90 = param_1;
  sVar9 = strlen(param_1);
  local_88 = sVar9 + 1;
  uVar10 = ((sVar9 + 0x18) / 0x10) * 0x10;
  for (plVar14 = &local_e8; plVar14 != (long *)((long)&local_e8 - (uVar10 & 0xfffffffffffff000));
      plVar14 = (long *)((long)plVar14 + -0x1000)) {
    *(undefined8 *)((long)plVar14 + -8) = *(undefined8 *)((long)plVar14 + -8);
  }
  lVar3 = -(ulong)((uint)uVar10 & 0xfff);
  if ((uVar10 & 0xfff) != 0) {
    *(undefined8 *)((long)plVar14 + ((ulong)((uint)uVar10 & 0xfff) - 8) + lVar3) =
         *(undefined8 *)((long)plVar14 + ((ulong)((uint)uVar10 & 0xfff) - 8) + lVar3);
  }
  sVar9 = local_88;
  pcVar5 = local_90;
  local_80 = (void *)((ulong)((long)plVar14 + lVar3 + 0xf) & 0xfffffffffffffff0);
  *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x1007dd;
  local_78 = memcpy((void *)((ulong)((long)plVar14 + lVar3 + 0xf) & 0xfffffffffffffff0),pcVar5,sVar9
                   );
  local_70 = (long)local_78 + (local_c8 - (long)local_c0);
  local_68 = (char *)((long)local_78 + (local_d8 - (long)local_c0));
  local_98 = local_e0;
  do {
    lVar4 = local_98;
    if (local_98 == 0) {
      uVar13 = 1;
LAB_00100b1f:
      if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100b33;
        __stack_chk_fail();
      }
      return uVar13;
    }
    *(undefined *)((long)local_78 + *(long *)(local_98 + 0x98)) = 0;
    if (*(char *)(local_e8 + 0x21) != '\0') {
      *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100866;
      tVar15 = (timespec)get_stat_atime(lVar4);
      lVar4 = local_98;
      local_48 = tVar15;
      *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x10087d;
      auVar16 = get_stat_mtime(lVar4);
      pcVar5 = local_68;
      iVar8 = local_cc;
      local_38._0_16_ = auVar16;
      *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x10089f;
      iVar8 = utimensat(iVar8,pcVar5,&local_48,0);
      pvVar6 = local_78;
      if (iVar8 != 0) {
        *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x1008b4;
        uVar13 = quotearg_style(4,pvVar6);
        *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x1008c6;
        uVar11 = gettext("failed to preserve times for %s");
        *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x1008ce;
        piVar12 = __errno_location();
        iVar8 = *piVar12;
        local_b0 = 0;
        local_ac = iVar8;
        local_60 = uVar11;
        *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100903;
        error(0,iVar8,uVar11,uVar13);
        uVar13 = 0;
        goto LAB_00100b1f;
      }
    }
    pcVar5 = local_68;
    iVar8 = local_cc;
    if (*(char *)(local_e8 + 0x1f) != '\0') {
      uVar1 = *(undefined4 *)(local_98 + 0x20);
      uVar2 = *(undefined4 *)(local_98 + 0x1c);
      *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100946;
      iVar8 = lchownat(iVar8,pcVar5,uVar2,uVar1);
      lVar4 = local_e8;
      if (iVar8 != 0) {
        *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x10095d;
        cVar7 = chown_failure_ok(lVar4);
        pcVar5 = local_68;
        pvVar6 = local_78;
        iVar8 = local_cc;
        if (cVar7 != '\x01') {
          *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100975;
          uVar13 = quotearg_style(4,pvVar6);
          *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100987;
          uVar11 = gettext("failed to preserve ownership for %s");
          *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x10098f;
          piVar12 = __errno_location();
          iVar8 = *piVar12;
          local_a8 = 0;
          local_a4 = iVar8;
          local_58 = uVar11;
          *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x1009c4;
          error(0,iVar8,uVar11,uVar13);
          uVar13 = 0;
          goto LAB_00100b1f;
        }
        uVar1 = *(undefined4 *)(local_98 + 0x20);
        *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x1009f1;
        local_b4 = lchownat(iVar8,pcVar5,0xffffffff,uVar1);
      }
    }
    pcVar5 = local_68;
    lVar4 = local_70;
    pvVar6 = local_78;
    iVar8 = local_cc;
    if (*(char *)(local_e8 + 0x20) == '\0') {
      if (*(char *)(local_98 + 0x90) != '\0') {
        uVar1 = *(undefined4 *)(local_98 + 0x18);
        *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100a73;
        iVar8 = lchmodat(iVar8,pcVar5,uVar1);
        pvVar6 = local_78;
        if (iVar8 != 0) {
          *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100a88;
          uVar13 = quotearg_style(4,pvVar6);
          *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100a9a;
          uVar11 = gettext("failed to preserve permissions for %s");
          *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100aa2;
          piVar12 = __errno_location();
          iVar8 = *piVar12;
          local_a0 = 0;
          local_9c = iVar8;
          local_50 = uVar11;
          *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100ad7;
          error(0,iVar8,uVar11,uVar13);
          uVar13 = 0;
          goto LAB_00100b1f;
        }
      }
    }
    else {
      uVar1 = *(undefined4 *)(local_98 + 0x18);
      *(undefined8 *)((long)plVar14 + lVar3 + -8) = 0x100a2d;
      iVar8 = xcopy_acl(lVar4,0xffffffff,pvVar6,0xffffffff,uVar1);
      if (iVar8 != 0) {
        uVar13 = 0;
        goto LAB_00100b1f;
      }
    }
    *(undefined *)((long)local_78 + *(long *)(local_98 + 0x98)) = 0x2f;
    local_98 = *(long *)(local_98 + 0xa0);
  } while( true );
}

// Function: make_dir_parents_private
undefined8
make_dir_parents_private
          (char *param_1,ulong param_2,int param_3,char *param_4,__dev_t **param_5,char *param_6,
          long param_7) {
  long lVar1;
  long lVar2;
  char *pcVar3;
  void *pvVar4;
  char *pcVar5;
  char cVar6;
  int iVar7;
  undefined4 uVar8;
  uint uVar9;
  undefined8 uVar10;
  size_t sVar11;
  ulong uVar12;
  int *piVar13;
  undefined8 uVar14;
  undefined *puVar15;
  undefined *puVar16;
  undefined *puVar17;
  long in_FS_OFFSET;
  undefined auStack_258 [8];
  long local_250;
  char *local_248;
  __dev_t **local_240;
  char *local_238;
  int local_22c;
  ulong local_228;
  char *local_220;
  char local_20d;
  uint local_20c;
  int local_208;
  uint local_204;
  uint local_200;
  uint local_1fc;
  undefined4 local_1f8;
  int local_1f4;
  undefined4 local_1f0;
  int local_1ec;
  undefined4 local_1e8;
  int local_1e4;
  undefined4 local_1e0;
  int local_1dc;
  undefined4 local_1d8;
  undefined4 local_1d4;
  undefined4 local_1d0;
  undefined4 local_1cc;
  char *local_1c8;
  char *local_1c0;
  __dev_t *local_1b8;
  ulong local_1b0;
  char *local_1a8;
  size_t local_1a0;
  void *local_198;
  void *local_190;
  char *local_188;
  void *local_180;
  undefined8 local_178;
  undefined8 local_170;
  undefined8 local_168;
  undefined8 local_160;
  undefined8 local_158;
  undefined8 local_150;
  stat local_148;
  stat local_b8;
  long local_20;
  
  puVar15 = auStack_258;
  local_250 = param_7;
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_248 = param_6;
  local_240 = param_5;
  local_238 = param_4;
  local_22c = param_3;
  local_228 = param_2;
  local_220 = param_1;
  local_1b0 = dir_len(param_1);
  *local_240 = (__dev_t *)0x0;
  if (local_228 < local_1b0) {
    local_1a8 = local_220;
    sVar11 = strlen(local_220);
    local_1a0 = sVar11 + 1;
    uVar12 = ((sVar11 + 0x18) / 0x10) * 0x10;
    for (; puVar15 != auStack_258 + -(uVar12 & 0xfffffffffffff000); puVar15 = puVar15 + -0x1000) {
      *(undefined8 *)(puVar15 + -8) = *(undefined8 *)(puVar15 + -8);
    }
    lVar1 = -(ulong)((uint)uVar12 & 0xfff);
    puVar16 = puVar15 + lVar1;
    if ((uVar12 & 0xfff) != 0) {
      *(undefined8 *)(puVar15 + ((ulong)((uint)uVar12 & 0xfff) - 8) + lVar1) =
           *(undefined8 *)(puVar15 + ((ulong)((uint)uVar12 & 0xfff) - 8) + lVar1);
    }
    sVar11 = local_1a0;
    pcVar3 = local_1a8;
    local_198 = (void *)((ulong)(puVar15 + lVar1 + 0xf) & 0xfffffffffffffff0);
    *(undefined8 *)(puVar15 + lVar1 + -8) = 0x100ca6;
    local_190 = memcpy((void *)((ulong)(puVar15 + lVar1 + 0xf) & 0xfffffffffffffff0),pcVar3,sVar11);
    local_188 = (char *)(local_228 + (long)local_190);
    uVar12 = ((local_1b0 + 0x18) / 0x10) * 0x10;
    for (; puVar16 != puVar15 + (lVar1 - (uVar12 & 0xfffffffffffff000)); puVar16 = puVar16 + -0x1000
        ) {
      *(undefined8 *)(puVar16 + -8) = *(undefined8 *)(puVar16 + -8);
    }
    lVar1 = -(ulong)((uint)uVar12 & 0xfff);
    puVar17 = puVar16 + lVar1;
    if ((uVar12 & 0xfff) != 0) {
      *(undefined8 *)(puVar16 + ((ulong)((uint)uVar12 & 0xfff) - 8) + lVar1) =
           *(undefined8 *)(puVar16 + ((ulong)((uint)uVar12 & 0xfff) - 8) + lVar1);
    }
    pvVar4 = local_190;
    sVar11 = local_1b0;
    local_180 = (void *)((ulong)(puVar16 + lVar1 + 0xf) & 0xfffffffffffffff0);
    *(undefined8 *)(puVar16 + lVar1 + -8) = 0x100d78;
    memcpy((void *)((ulong)(puVar16 + lVar1 + 0xf) & 0xfffffffffffffff0),pvVar4,sVar11);
    iVar7 = local_22c;
    *(undefined *)((long)local_180 + local_1b0) = 0;
    for (local_1c8 = (char *)(local_228 + (long)local_180); pcVar3 = local_1c8, *local_1c8 == '/';
        local_1c8 = local_1c8 + 1) {
    }
    *(undefined8 *)(puVar16 + lVar1 + -8) = 0x100ddc;
    iVar7 = fstatat(iVar7,pcVar3,&local_148,0);
    pvVar4 = local_180;
    if (iVar7 == 0) {
      if ((local_148.st_mode & 0xf000) != 0x4000) {
        *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101562;
        uVar10 = quotearg_style(4,pvVar4);
        *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101574;
        uVar14 = gettext("%s exists but is not a directory");
        local_1d0 = 0;
        local_1cc = 0;
        local_150 = uVar14;
        *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1015b1;
        error(0,0,uVar14,uVar10);
        uVar10 = 0;
        goto LAB_001015c8;
      }
      *local_248 = '\0';
    }
    else {
      for (local_1c0 = local_188; *local_1c0 == '/'; local_1c0 = local_1c0 + 1) {
      }
      local_1c8 = local_1c0;
      while( true ) {
        pcVar3 = local_1c0;
        *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101522;
        local_1c0 = strchr(pcVar3,0x2f);
        pcVar3 = local_1c8;
        iVar7 = local_22c;
        if (local_1c0 == (char *)0x0) break;
        *local_1c0 = '\0';
        *(undefined8 *)(puVar16 + lVar1 + -8) = 0x100e47;
        iVar7 = fstatat(iVar7,pcVar3,&local_148,0);
        pcVar3 = local_188;
        local_20d = iVar7 != 0;
        if (((((bool)local_20d) || (*(char *)(local_250 + 0x1f) != '\0')) ||
            (*(char *)(local_250 + 0x20) != '\0')) || (*(char *)(local_250 + 0x21) != '\0')) {
          *(undefined8 *)(puVar16 + lVar1 + -8) = 0x100ea5;
          iVar7 = stat(pcVar3,&local_b8);
          if (iVar7 == 0) {
            if ((local_b8.st_mode & 0xf000) == 0x4000) {
              local_208 = 0;
            }
            else {
              local_208 = 0x14;
            }
          }
          else {
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x100eae;
            piVar13 = __errno_location();
            local_208 = *piVar13;
          }
          pcVar3 = local_188;
          if (local_208 != 0) {
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x100ef3;
            uVar10 = quotearg_style(4,pcVar3);
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x100f05;
            uVar14 = gettext("failed to get attributes of %s");
            iVar7 = local_208;
            local_1f8 = 0;
            local_1f4 = local_208;
            local_178 = uVar14;
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x100f44;
            error(0,iVar7,uVar14,uVar10);
            uVar10 = 0;
            puVar17 = puVar16 + lVar1;
            goto LAB_001015c8;
          }
          *(undefined8 *)(puVar16 + lVar1 + -8) = 0x100f59;
          local_1b8 = (__dev_t *)xmalloc(0xa8);
          *local_1b8 = local_b8.st_dev;
          local_1b8[1] = local_b8.st_ino;
          local_1b8[2] = local_b8.st_nlink;
          local_1b8[3] = CONCAT44(local_b8.st_uid,local_b8.st_mode);
          local_1b8[4] = local_b8._32_8_;
          local_1b8[5] = local_b8.st_rdev;
          local_1b8[6] = local_b8.st_size;
          local_1b8[7] = local_b8.st_blksize;
          local_1b8[8] = local_b8.st_blocks;
          local_1b8[9] = local_b8.st_atim.tv_sec;
          local_1b8[10] = local_b8.st_atim.tv_nsec;
          local_1b8[0xb] = local_b8.st_mtim.tv_sec;
          local_1b8[0xc] = local_b8.st_mtim.tv_nsec;
          local_1b8[0xd] = local_b8.st_ctim.tv_sec;
          local_1b8[0xe] = local_b8.st_ctim.tv_nsec;
          local_1b8[0xf] = local_b8.__unused[0];
          local_1b8[0x10] = local_b8.__unused[1];
          local_1b8[0x11] = local_b8.__unused[2];
          local_1b8[0x13] = (long)local_1c0 - (long)local_190;
          *(undefined *)(local_1b8 + 0x12) = 0;
          local_1b8[0x14] = (__dev_t)*local_240;
          *local_240 = local_1b8;
        }
        pcVar3 = local_188;
        pvVar4 = local_190;
        cVar6 = local_20d;
        lVar2 = local_250;
        if (local_20d == '\0') {
          uVar8 = 0;
        }
        else {
          uVar8 = *(undefined4 *)(local_1b8 + 3);
        }
        *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1010a6;
        cVar6 = set_process_security_ctx(pcVar3,pvVar4,uVar8,cVar6,lVar2);
        pvVar4 = local_190;
        pcVar3 = local_1c8;
        iVar7 = local_22c;
        if (cVar6 != '\x01') {
          uVar10 = 0;
          puVar17 = puVar16 + lVar1;
          goto LAB_001015c8;
        }
        if (local_20d == '\0') {
          if ((local_148.st_mode & 0xf000) != 0x4000) {
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101412;
            uVar10 = quotearg_style(4,pvVar4);
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101424;
            uVar14 = gettext("%s exists but is not a directory");
            local_1d8 = 0;
            local_1d4 = 0;
            local_158 = uVar14;
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101461;
            error(0,0,uVar14,uVar10);
            uVar10 = 0;
            puVar17 = puVar16 + lVar1;
            goto LAB_001015c8;
          }
          *local_248 = '\0';
        }
        else {
          *local_248 = '\x01';
          local_204 = *(uint *)(local_1b8 + 3);
          if (*(char *)(local_250 + 0x1f) == '\x01') {
            local_20c = 0x3f;
          }
          else if (*(char *)(local_250 + 0x20) == '\0') {
            local_20c = 0;
          }
          else {
            local_20c = 0x12;
          }
          local_20c = local_20c & local_204;
          uVar9 = local_204;
          if (*(char *)(local_250 + 0x22) != '\0') {
            uVar9 = 0x1ff;
          }
          uVar9 = ~local_20c & uVar9 & 0xfff;
          local_200 = uVar9;
          *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101176;
          iVar7 = mkdirat(iVar7,pcVar3,uVar9);
          pcVar5 = local_188;
          pvVar4 = local_190;
          pcVar3 = local_238;
          if (iVar7 != 0) {
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x10118e;
            uVar10 = quotearg_style(4,pvVar4);
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1011a0;
            uVar14 = gettext("cannot make directory %s");
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1011a8;
            piVar13 = __errno_location();
            iVar7 = *piVar13;
            local_1f0 = 0;
            local_1ec = iVar7;
            local_170 = uVar14;
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1011e3;
            error(0,iVar7,uVar14,uVar10);
            uVar10 = 0;
            puVar17 = puVar16 + lVar1;
            goto LAB_001015c8;
          }
          if (local_238 != (char *)0x0) {
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x10121d;
            printf(pcVar3,pcVar5,pvVar4);
          }
          pcVar3 = local_1c8;
          iVar7 = local_22c;
          *(undefined8 *)(puVar16 + lVar1 + -8) = 0x10123d;
          iVar7 = fstatat(iVar7,pcVar3,&local_148,0x100);
          pvVar4 = local_190;
          if (iVar7 != 0) {
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101255;
            uVar10 = quotearg_style(4,pvVar4);
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101267;
            uVar14 = gettext("failed to get attributes of %s");
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x10126f;
            piVar13 = __errno_location();
            iVar7 = *piVar13;
            local_1e8 = 0;
            local_1e4 = iVar7;
            local_168 = uVar14;
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1012aa;
            error(0,iVar7,uVar14,uVar10);
            uVar10 = 0;
            puVar17 = puVar16 + lVar1;
            goto LAB_001015c8;
          }
          if (*(char *)(local_250 + 0x20) != '\x01') {
            if ((~local_148.st_mode & local_20c) != 0) {
              *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1012de;
              uVar9 = cached_umask();
              local_20c = local_20c & ~uVar9;
            }
            if (((~local_148.st_mode & local_20c) != 0) || ((local_148.st_mode & 0x1c0) != 0x1c0)) {
              *(__mode_t *)(local_1b8 + 3) = local_148.st_mode | local_20c;
              *(undefined *)(local_1b8 + 0x12) = 1;
            }
          }
          pcVar3 = local_1c8;
          iVar7 = local_22c;
          uVar9 = local_148.st_mode | 0x1c0;
          local_1fc = uVar9;
          if (uVar9 != local_148.st_mode) {
            *(undefined8 *)(puVar16 + lVar1 + -8) = 0x101370;
            iVar7 = lchmodat(iVar7,pcVar3,uVar9);
            pvVar4 = local_190;
            if (iVar7 != 0) {
              *(undefined8 *)(puVar16 + lVar1 + -8) = 0x10138c;
              uVar10 = quotearg_style(4,pvVar4);
              *(undefined8 *)(puVar16 + lVar1 + -8) = 0x10139e;
              uVar14 = gettext("setting permissions for %s");
              *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1013a6;
              piVar13 = __errno_location();
              iVar7 = *piVar13;
              local_1e0 = 0;
              local_1dc = iVar7;
              local_160 = uVar14;
              *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1013e1;
              error(0,iVar7,uVar14,uVar10);
              uVar10 = 0;
              puVar17 = puVar16 + lVar1;
              goto LAB_001015c8;
            }
          }
        }
        pvVar4 = local_190;
        lVar2 = local_250;
        if ((*local_248 != '\x01') &&
           ((*(long *)(local_250 + 0x28) != 0 || (*(char *)(local_250 + 0x33) != '\0')))) {
          *(undefined8 *)(puVar16 + lVar1 + -8) = 0x1014c1;
          cVar6 = set_file_security_ctx(pvVar4,0,lVar2);
          if ((cVar6 != '\x01') && (*(char *)(local_250 + 0x34) != '\0')) {
            uVar10 = 0;
            puVar17 = puVar16 + lVar1;
            goto LAB_001015c8;
          }
        }
        *local_1c0 = '/';
        do {
          local_1c0 = local_1c0 + 1;
        } while (*local_1c0 == '/');
      }
    }
    uVar10 = 1;
    puVar17 = puVar16 + lVar1;
  }
  else {
    uVar10 = 1;
    puVar17 = auStack_258;
  }
LAB_001015c8:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar10;
  }
                    /* WARNING: Subroutine does not return */
  *(undefined8 *)(puVar17 + -8) = 0x1015dc;
  __stack_chk_fail();
}

// Function: do_copy
byte do_copy(int param_1,EVP_PKEY_CTX **param_2,EVP_PKEY_CTX *param_3,char param_4,ulong *param_5) {
  long lVar1;
  ulong *puVar2;
  undefined4 uVar3;
  char *pcVar4;
  EVP_PKEY_CTX *dst;
  char cVar5;
  byte bVar6;
  uint uVar7;
  int iVar8;
  undefined8 uVar9;
  undefined8 uVar10;
  int *piVar11;
  size_t sVar12;
  ulong uVar13;
  void *pvVar14;
  EVP_PKEY_CTX *pEVar15;
  long lVar16;
  char *pcVar17;
  undefined *puVar18;
  ulong **ppuVar19;
  long in_FS_OFFSET;
  ulong *local_1e8;
  EVP_PKEY_CTX *local_1e0;
  EVP_PKEY_CTX **local_1d8;
  char local_1d0;
  int local_1cc;
  char local_1c4;
  undefined local_1c3;
  byte local_1c2;
  char local_1c1;
  undefined4 local_1c0;
  int local_1bc;
  undefined4 local_1b8;
  int local_1b4;
  undefined4 local_1b0;
  undefined4 local_1ac;
  undefined4 local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  undefined4 local_19c;
  undefined4 local_198;
  undefined4 local_194;
  undefined4 local_190;
  int local_18c;
  undefined4 local_188;
  int local_184;
  undefined4 local_180;
  undefined4 local_17c;
  void *local_178;
  char *local_170;
  EVP_PKEY_CTX *local_168;
  char *local_160;
  EVP_PKEY_CTX *local_158;
  EVP_PKEY_CTX *local_150;
  EVP_PKEY_CTX *local_148;
  EVP_PKEY_CTX *local_140;
  char *local_138;
  size_t local_130;
  void *local_128;
  void *local_120;
  EVP_PKEY_CTX *local_118;
  size_t local_110;
  void *local_108;
  void *local_100;
  void *local_f8;
  undefined8 local_f0;
  undefined8 local_e8;
  undefined8 local_e0;
  undefined8 local_d8;
  undefined8 local_d0;
  undefined8 local_c8;
  undefined8 local_c0;
  stat local_b8;
  long local_20;
  
  ppuVar19 = &local_1e8;
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_1c4 = '\0';
  local_1c2 = 1;
  local_1e8 = param_5;
  local_1e0 = param_3;
  local_1d8 = param_2;
  local_1d0 = param_4;
  local_1cc = param_1;
  if (param_1 <= (int)(uint)(param_3 == (EVP_PKEY_CTX *)0x0)) {
    if (param_1 < 1) {
      local_f0 = gettext("missing file operand");
      local_1b0 = 0;
      local_1ac = 0;
      error(0,0,local_f0);
    }
    else {
      uVar9 = quotearg_style(4,*param_2);
      local_e8 = gettext("missing destination file operand after %s");
      local_1a8 = 0;
      local_1a4 = 0;
      error(0,0,local_e8,uVar9);
    }
    usage(1);
  }
  local_b8.st_mode = 0;
  local_1c0 = 0xffffff9c;
  if (local_1d0 != '\0') {
    if (local_1e0 != (EVP_PKEY_CTX *)0x0) {
      local_e0 = gettext("cannot combine --target-directory (-t) and --no-target-directory (-T)");
      local_1a0 = 1;
      local_19c = 0;
      error(1,0,local_e0);
    }
    if (local_1cc < 3) goto LAB_001019c6;
    uVar9 = quotearg_style(4,local_1d8[2]);
    local_d8 = gettext("extra operand %s");
    local_198 = 0;
    local_194 = 0;
    error(0,0,local_d8,uVar9);
    usage(1);
  }
  if (local_1e0 != (EVP_PKEY_CTX *)0x0) {
    local_1c0 = target_directory_operand(local_1e0,&local_b8);
    cVar5 = target_dirfd_valid(local_1c0);
    if (cVar5 == '\x01') goto LAB_001019c6;
    uVar9 = quotearg_style(4,local_1e0);
    uVar10 = gettext("target directory %s");
    piVar11 = __errno_location();
    local_18c = *piVar11;
    local_190 = 1;
    local_d0 = uVar10;
    error(1,local_18c,uVar10,uVar9);
  }
  local_150 = local_1d8[(long)local_1cc + -1];
  local_1b8 = target_directory_operand(local_150,&local_b8);
  cVar5 = target_dirfd_valid(local_1b8);
  if (cVar5 == '\0') {
    piVar11 = __errno_location();
    local_1b4 = *piVar11;
    if (local_1b4 == 2) {
      local_1c4 = '\x01';
    }
    if (2 < local_1cc) {
      uVar9 = quotearg_style(4,local_150);
      local_c8 = gettext("target %s");
      local_188 = 1;
      local_184 = local_1b4;
      error(1,local_1b4,local_c8,uVar9);
    }
  }
  else {
    local_1c0 = local_1b8;
    local_1e0 = local_150;
    local_1cc = local_1cc + -1;
  }
LAB_001019c6:
  if (local_1e0 == (EVP_PKEY_CTX *)0x0) {
    local_148 = *local_1d8;
    local_158 = local_1d8[1];
    if (parents_option != '\0') {
      local_c0 = gettext("with --parents, the destination must be a directory");
      local_180 = 0;
      local_17c = 0;
      error(0,0,local_c0);
      usage(1);
    }
    if (((((*(char *)((long)local_1e8 + 0x16) != '\0') && (*(int *)local_1e8 != 0)) &&
         (cVar5 = streq(local_148,local_158), cVar5 != '\0')) &&
        ((local_1c4 != '\x01' &&
         ((local_b8.st_mode != 0 || (iVar8 = stat((char *)local_158,&local_b8), iVar8 == 0)))))) &&
       ((local_b8.st_mode & 0xf000) == 0x8000)) {
      local_158 = (EVP_PKEY_CTX *)find_backup_file_name(0xffffff9c,local_158,*(int *)local_1e8);
      x_tmp_3._8_8_ = local_1e8[1];
      x_tmp_3._16_8_ = local_1e8[2];
      x_tmp_3._24_8_ = local_1e8[3];
      x_tmp_3._32_8_ = local_1e8[4];
      x_tmp_3._40_8_ = local_1e8[5];
      x_tmp_3._48_8_ = local_1e8[6];
      x_tmp_3._56_8_ = local_1e8[7];
      x_tmp_3._64_8_ = local_1e8[8];
      x_tmp_3._72_8_ = local_1e8[9];
      x_tmp_3._88_8_ = local_1e8[0xb];
      x_tmp_3._80_8_ = local_1e8[10];
      x_tmp_3._0_8_ = *local_1e8 & 0xffffffff00000000;
      local_1e8 = (ulong *)x_tmp_3;
    }
    iVar8 = copy(local_148,local_158);
    local_1c2 = (byte)iVar8;
    ppuVar19 = &local_1e8;
  }
  else {
    if (1 < local_1cc) {
      dest_info_init(local_1e8);
      src_info_init(local_1e8);
    }
    for (local_1bc = 0; local_1bc < local_1cc; local_1bc = local_1bc + 1) {
      local_1c1 = '\x01';
      pEVar15 = local_1d8[local_1bc];
      local_140 = pEVar15;
      if (remove_trailing_slashes != '\0') {
        *(undefined8 *)((long)ppuVar19 + -8) = 0x101a4f;
        strip_trailing_slashes(pEVar15);
      }
      pEVar15 = local_140;
      if (parents_option == '\0') {
        *(undefined8 *)((long)ppuVar19 + -8) = 0x101bf4;
        pcVar17 = (char *)last_component(pEVar15);
        local_138 = pcVar17;
        *(undefined8 *)((long)ppuVar19 + -8) = 0x101c0a;
        sVar12 = strlen(pcVar17);
        local_130 = sVar12 + 1;
        uVar13 = ((sVar12 + 0x18) / 0x10) * 0x10;
        for (puVar18 = (undefined *)ppuVar19;
            puVar18 != (undefined *)((long)ppuVar19 + -(uVar13 & 0xfffffffffffff000));
            puVar18 = puVar18 + -0x1000) {
          *(undefined8 *)(puVar18 + -8) = *(undefined8 *)(puVar18 + -8);
        }
        lVar1 = -(ulong)((uint)uVar13 & 0xfff);
        ppuVar19 = (ulong **)(puVar18 + lVar1);
        if ((uVar13 & 0xfff) != 0) {
          *(undefined8 *)(puVar18 + ((ulong)((uint)uVar13 & 0xfff) - 8) + lVar1) =
               *(undefined8 *)(puVar18 + ((ulong)((uint)uVar13 & 0xfff) - 8) + lVar1);
        }
        sVar12 = local_130;
        pcVar17 = local_138;
        local_128 = (void *)((ulong)(puVar18 + lVar1 + 0xf) & 0xfffffffffffffff0);
        *(undefined8 *)(puVar18 + lVar1 + -8) = 0x101cc4;
        pvVar14 = memcpy((void *)((ulong)(puVar18 + lVar1 + 0xf) & 0xfffffffffffffff0),pcVar17,
                         sVar12);
        local_120 = pvVar14;
        *(undefined8 *)(puVar18 + lVar1 + -8) = 0x101cda;
        strip_trailing_slashes(pvVar14);
        pvVar14 = local_120;
        *(undefined8 *)(puVar18 + lVar1 + -8) = 0x101cf3;
        bVar6 = streq(pvVar14,&DAT_00104aa7);
        pEVar15 = local_1e0;
        lVar16 = (long)local_120 + (ulong)bVar6;
        local_120 = (void *)lVar16;
        *(undefined8 *)(puVar18 + lVar1 + -8) = 0x101d1d;
        local_168 = (EVP_PKEY_CTX *)file_name_concat(pEVar15,lVar16,&local_170);
      }
      else {
        local_118 = local_140;
        *(undefined8 *)((long)ppuVar19 + -8) = 0x101a7b;
        sVar12 = strlen((char *)pEVar15);
        local_110 = sVar12 + 1;
        uVar13 = ((sVar12 + 0x18) / 0x10) * 0x10;
        for (puVar18 = (undefined *)ppuVar19;
            puVar18 != (undefined *)((long)ppuVar19 + -(uVar13 & 0xfffffffffffff000));
            puVar18 = puVar18 + -0x1000) {
          *(undefined8 *)(puVar18 + -8) = *(undefined8 *)(puVar18 + -8);
        }
        lVar1 = -(ulong)((uint)uVar13 & 0xfff);
        ppuVar19 = (ulong **)(puVar18 + lVar1);
        if ((uVar13 & 0xfff) != 0) {
          *(undefined8 *)(puVar18 + ((ulong)((uint)uVar13 & 0xfff) - 8) + lVar1) =
               *(undefined8 *)(puVar18 + ((ulong)((uint)uVar13 & 0xfff) - 8) + lVar1);
        }
        sVar12 = local_110;
        pEVar15 = local_118;
        local_108 = (void *)((ulong)(puVar18 + lVar1 + 0xf) & 0xfffffffffffffff0);
        *(undefined8 *)(puVar18 + lVar1 + -8) = 0x101b35;
        pvVar14 = memcpy((void *)((ulong)(puVar18 + lVar1 + 0xf) & 0xfffffffffffffff0),pEVar15,
                         sVar12);
        local_100 = pvVar14;
        *(undefined8 *)(puVar18 + lVar1 + -8) = 0x101b4b;
        strip_trailing_slashes(pvVar14);
        pvVar14 = local_100;
        pEVar15 = local_1e0;
        *(undefined8 *)(puVar18 + lVar1 + -8) = 0x101b6b;
        pEVar15 = (EVP_PKEY_CTX *)file_name_concat(pEVar15,pvVar14,&local_170);
        uVar3 = local_1c0;
        if (*(char *)(local_1e8 + 8) == '\0') {
          pcVar17 = (char *)0x0;
        }
        else {
          pcVar17 = "%s -> %s\n";
        }
        lVar16 = (long)local_170 - (long)pEVar15;
        local_168 = pEVar15;
        *(ulong **)(puVar18 + lVar1 + -0x10) = local_1e8;
        *(undefined8 *)(puVar18 + lVar1 + -0x18) = 0x101bd6;
        local_1c1 = make_dir_parents_private(pEVar15,lVar16,uVar3,pcVar17,&local_178,&local_1c4);
      }
      dst = local_140;
      pEVar15 = local_168;
      if (local_1c1 == '\x01') {
        for (local_160 = local_170; *local_160 == '/'; local_160 = local_160 + 1) {
        }
        *(undefined8 *)((long)ppuVar19 + -8) = 0;
        *(undefined **)((long)ppuVar19 + -0x10) = &local_1c3;
        *(undefined8 *)((long)ppuVar19 + -0x18) = 0x101da3;
        uVar7 = copy(dst,pEVar15);
        pcVar4 = local_160;
        pEVar15 = local_168;
        pcVar17 = local_170;
        pvVar14 = local_178;
        uVar3 = local_1c0;
        puVar2 = local_1e8;
        local_1c2 = (uVar7 & 0xff & (uint)local_1c2) != 0;
        if (parents_option != '\0') {
          *(undefined8 *)((long)ppuVar19 + -8) = 0x101e00;
          bVar6 = re_protect(pEVar15,pcVar17,uVar3,pcVar4,pvVar14,puVar2);
          local_1c2 = (bVar6 & local_1c2) != 0;
        }
      }
      else {
        local_1c2 = false;
      }
      if (parents_option != '\0') {
        while (pvVar14 = local_178, local_178 != (void *)0x0) {
          local_f8 = local_178;
          local_178 = *(void **)((long)local_178 + 0xa0);
          *(undefined8 *)((long)ppuVar19 + -8) = 0x101e56;
          free(pvVar14);
        }
      }
      pEVar15 = local_168;
      *(undefined8 *)((long)ppuVar19 + -8) = 0x101e71;
      free(pEVar15);
    }
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_1c2;
  }
                    /* WARNING: Subroutine does not return */
  *(undefined8 *)((long)ppuVar19 + -8) = 0x1020de;
  __stack_chk_fail();
}

// Function: cp_option_init
void cp_option_init(long param_1) {
  char *pcVar1;
  
  cp_options_default(param_1);
  *(undefined *)(param_1 + 0x14) = 1;
  *(undefined4 *)(param_1 + 4) = 1;
  *(undefined *)(param_1 + 0x15) = 0;
  *(undefined *)(param_1 + 0x16) = 0;
  *(undefined *)(param_1 + 0x17) = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  *(undefined *)(param_1 + 0x18) = 0;
  *(undefined *)(param_1 + 0x1b) = 0;
  *(undefined *)(param_1 + 0x1e) = 0;
  *(undefined4 *)(param_1 + 0x4c) = 1;
  *(undefined *)(param_1 + 0x1f) = 0;
  *(undefined *)(param_1 + 0x30) = 0;
  *(undefined *)(param_1 + 0x20) = 0;
  *(undefined *)(param_1 + 0x21) = 0;
  *(undefined *)(param_1 + 0x22) = 0;
  *(undefined *)(param_1 + 0x33) = 0;
  *(undefined *)(param_1 + 0x34) = 0;
  *(undefined8 *)(param_1 + 0x28) = 0;
  *(undefined *)(param_1 + 0x35) = 0;
  *(undefined *)(param_1 + 0x37) = 0;
  *(undefined *)(param_1 + 0x36) = 0;
  *(undefined *)(param_1 + 0x31) = 1;
  *(undefined *)(param_1 + 0x32) = 0;
  *(undefined *)(param_1 + 0x38) = 0;
  *(undefined4 *)(param_1 + 0xc) = 2;
  *(undefined *)(param_1 + 0x3a) = 0;
  *(undefined *)(param_1 + 0x39) = 0;
  *(undefined4 *)(param_1 + 0x10) = 0;
  *(undefined *)(param_1 + 0x43) = 0;
  *(undefined4 *)(param_1 + 0x3c) = 0;
  *(undefined *)(param_1 + 0x40) = 0;
  *(undefined *)(param_1 + 0x41) = 0;
  pcVar1 = getenv("POSIXLY_CORRECT");
  *(bool *)(param_1 + 0x44) = pcVar1 != (char *)0x0;
  *(undefined8 *)(param_1 + 0x50) = 0;
  *(undefined8 *)(param_1 + 0x58) = 0;
  return;
}

// Function: decode_preserve_arg
void decode_preserve_arg(undefined8 param_1,long param_2,char param_3) {
  char *__ptr;
  char *pcVar1;
  long lVar2;
  char *local_20;
  char *local_18;
  
  __ptr = (char *)xstrdup(param_1);
  local_20 = __ptr;
  do {
    pcVar1 = strchr(local_20,0x2c);
    local_18 = pcVar1;
    if (pcVar1 != (char *)0x0) {
      local_18 = pcVar1 + 1;
      *pcVar1 = '\0';
    }
    if (param_3 == '\0') {
      pcVar1 = "--no-preserve";
    }
    else {
      pcVar1 = "--preserve";
    }
    lVar2 = __xargmatch_internal(pcVar1,local_20,preserve_args_2,preserve_vals_1,4,_argmatch_die,1);
    switch(*(undefined4 *)(preserve_vals_1 + lVar2 * 4)) {
    case 0:
      *(char *)(param_2 + 0x20) = param_3;
      *(bool *)(param_2 + 0x22) = param_3 == '\0';
      break;
    case 1:
      *(char *)(param_2 + 0x21) = param_3;
      break;
    case 2:
      *(char *)(param_2 + 0x1f) = param_3;
      break;
    case 3:
      *(char *)(param_2 + 0x30) = param_3;
      break;
    case 4:
      *(char *)(param_2 + 0x34) = param_3;
      *(char *)(param_2 + 0x33) = param_3;
      break;
    case 5:
      *(char *)(param_2 + 0x35) = param_3;
      *(char *)(param_2 + 0x36) = param_3;
      break;
    case 6:
      *(char *)(param_2 + 0x20) = param_3;
      *(char *)(param_2 + 0x21) = param_3;
      *(char *)(param_2 + 0x1f) = param_3;
      *(char *)(param_2 + 0x30) = param_3;
      *(bool *)(param_2 + 0x22) = param_3 == '\0';
      if (selinux_enabled != '\0') {
        *(char *)(param_2 + 0x33) = param_3;
      }
      *(char *)(param_2 + 0x35) = param_3;
      break;
    default:
                    /* WARNING: Subroutine does not return */
      __assert_fail("0",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/cp.c",
                    0x3c5,"decode_preserve_arg");
    }
    local_20 = local_18;
  } while (local_18 != (char *)0x0);
  free(__ptr);
  return;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  bool bVar1;
  byte bVar2;
  int iVar3;
  long lVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  int *piVar8;
  long in_FS_OFFSET;
  undefined local_14f;
  undefined local_14e;
  long local_f8;
  long local_f0;
  long local_e8;
  long local_e0;
  undefined4 local_88;
  int local_84;
  int local_80;
  int local_7c;
  undefined local_74;
  undefined local_73;
  undefined local_72;
  char local_71;
  undefined local_6a;
  undefined local_69;
  undefined local_68;
  undefined local_67;
  long local_60;
  undefined local_58;
  undefined local_57;
  undefined local_56;
  char local_55;
  char local_54;
  undefined local_53;
  char local_52;
  undefined local_51;
  char local_50;
  char local_4e;
  int local_4c;
  undefined local_48;
  undefined local_47;
  undefined local_46;
  int local_3c;
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  bVar1 = false;
  local_f8 = 0;
  local_f0 = 0;
  local_14f = 0;
  local_e8 = 0;
  local_14e = 0;
  local_e0 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdin);
  selinux_enabled = '\0';
  cp_option_init(&local_88);
LAB_00102ac6:
  do {
    iVar3 = getopt_long(param_1,param_2,"abdfHilLnprst:uvxPRS:TZ",long_opts,0);
    if (iVar3 == -1) {
      if ((local_3c == 1) && (local_7c == 1)) {
        local_3c = 0;
      }
      if ((local_71 != '\0') && (local_4e != '\0')) {
        uVar6 = gettext("cannot make both hard and symbolic links");
        error(0,0,uVar6);
        usage(1);
      }
      if (local_80 == 2) {
        local_4c = 2;
      }
      if ((bVar1) && ((local_4c == 2 || (local_4c == 3)))) {
        uVar6 = gettext("--backup is mutually exclusive with -n or --update=none-fail");
        error(0,0,uVar6);
        usage(1);
      }
      if ((local_3c == 2) && (local_7c != 2)) {
        uVar6 = gettext("--reflink can be used only with --sparse=auto");
        error(0,0,uVar6);
        usage(1);
      }
      if (bVar1) {
        uVar6 = gettext("backup type");
        local_88 = xget_version(uVar6,local_f0);
      }
      else {
        local_88 = 0;
      }
      set_simple_backup_suffix(local_f8);
      if (local_84 == 1) {
        if ((local_50 == '\0') || (local_71 == '\x01')) {
          local_84 = 4;
        }
        else {
          local_84 = 2;
        }
      }
      if (local_50 != '\0') {
        local_74 = local_14f;
      }
      if (((local_60 != 0) || (local_e0 != 0)) && (local_54 != '\x01')) {
        local_55 = '\0';
      }
      if ((local_55 != '\0') && ((local_60 != 0 || (local_e0 != 0)))) {
        uVar6 = gettext("cannot set target context and preserve it");
        error(1,0,uVar6);
      }
      if ((local_54 != '\0') && (selinux_enabled != '\x01')) {
        uVar6 = gettext("cannot preserve security context without an SELinux-enabled kernel");
        error(1,0,uVar6);
      }
      if ((local_e0 != 0) && (iVar3 = setfscreatecon(local_e0), iVar3 < 0)) {
        uVar6 = quote(local_e0);
        uVar7 = gettext("failed to set default file creation context to %s");
        piVar8 = __errno_location();
        error(1,*piVar8,uVar7,uVar6);
      }
      if (local_52 != '\0') {
        uVar6 = gettext("cannot preserve extended attributes, cp is built without xattr support");
        error(1,0,uVar6);
      }
      hash_init();
      bVar2 = do_copy(param_1 - _optind,param_2 + _optind,local_e8,local_14e,&local_88);
                    /* WARNING: Subroutine does not return */
      exit((uint)(bVar2 ^ 1));
    }
    switch(iVar3) {
    case 0x48:
      local_84 = 3;
      break;
    case 0x4c:
      local_84 = 4;
      break;
    case 0x50:
      local_84 = 2;
      break;
    case 0x52:
    case 0x72:
      local_50 = '\x01';
      break;
    case 0x53:
      bVar1 = true;
      local_f8 = _optarg;
      break;
    case 0x54:
      local_14e = 1;
      break;
    case 0x5a:
      if (selinux_enabled == '\0') {
        if (_optarg != 0) {
          uVar6 = gettext("warning: ignoring --context; it requires an SELinux-enabled kernel");
          error(0,0,uVar6);
        }
      }
      else if (_optarg == 0) {
        local_60 = selabel_open(0,0,0);
        if (local_60 == 0) {
          uVar6 = gettext("warning: ignoring --context");
          piVar8 = __errno_location();
          error(0,*piVar8,uVar6);
        }
      }
      else {
        local_e0 = _optarg;
      }
      break;
    case 0x61:
      local_84 = 2;
      local_58 = 1;
      local_69 = 1;
      local_68 = 1;
      local_67 = 1;
      local_56 = 1;
      if (selinux_enabled != '\0') {
        local_55 = '\x01';
      }
      local_53 = 1;
      local_51 = 1;
      local_50 = '\x01';
      break;
    case 0x62:
      bVar1 = true;
      if (_optarg != 0) {
        local_f0 = _optarg;
      }
      break;
    case 100:
      local_58 = 1;
      local_84 = 2;
      break;
    case 0x66:
      local_72 = 1;
      break;
    case 0x69:
      local_80 = 3;
      break;
    case 0x6c:
      local_71 = '\x01';
      break;
    case 0x6e:
      local_80 = 2;
      break;
    case 0x73:
      local_4e = '\x01';
      break;
    case 0x74:
      if (local_e8 != 0) {
        uVar6 = gettext("multiple target directories specified");
        error(1,0,uVar6);
      }
      local_e8 = _optarg;
      break;
    case 0x75:
      local_4c = 1;
      if (_optarg != 0) {
        lVar4 = __xargmatch_internal
                          ("--update",_optarg,update_type_string,update_type,4,_argmatch_die,1);
        local_4c = *(int *)(update_type + lVar4 * 4);
      }
      break;
    case 0x76:
      local_48 = 1;
      break;
    case 0x78:
      local_6a = 1;
      break;
    case 0x80:
      local_57 = 0;
      break;
    case 0x81:
      local_14f = 1;
      break;
    case 0x82:
      local_48 = 1;
      local_46 = 1;
      break;
    case 0x83:
      decode_preserve_arg(_optarg,&local_88,0);
      break;
    case 0x84:
      parents_option = 1;
      break;
    case 0x85:
      if (_optarg != 0) {
        decode_preserve_arg(_optarg,&local_88,1);
        local_56 = 1;
        break;
      }
    case 0x70:
      local_69 = 1;
      local_68 = 1;
      local_67 = 1;
      local_56 = 1;
      break;
    case 0x86:
      if (_optarg == 0) {
        local_3c = 2;
      }
      else {
        lVar4 = __xargmatch_internal
                          ("--reflink",_optarg,reflink_type_string,reflink_type,4,_argmatch_die,1);
        local_3c = *(int *)(reflink_type + lVar4 * 4);
      }
      break;
    case 0x87:
      lVar4 = __xargmatch_internal
                        ("--sparse",_optarg,sparse_type_string,sparse_type,4,_argmatch_die,1);
      local_7c = *(int *)(sparse_type + lVar4 * 4);
      break;
    case 0x88:
      remove_trailing_slashes = 1;
      break;
    case 0x89:
      local_73 = 1;
      break;
    case 0x8a:
      goto switchD_00102597_caseD_8a;
    case -0x82:
      usage(0);
    case -0x83:
      uVar6 = proper_name_lite("Jim Meyering","Jim Meyering");
      uVar7 = proper_name_lite("David MacKenzie","David MacKenzie");
      uVar5 = proper_name_lite("Torbjorn Granlund",&DAT_00104c58);
      version_etc(_stdout,&DAT_001048f2,"GNU coreutils",_Version,uVar5,uVar7,uVar6,0);
                    /* WARNING: Subroutine does not return */
      exit(0);
    default:
      usage(1);
    }
  } while( true );
switchD_00102597_caseD_8a:
  local_47 = 1;
  goto LAB_00102ac6;
}

