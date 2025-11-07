// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... [TEMPLATE]\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Create a temporary file or directory, safely, and print its name.\nTEMPLATE must contain at least 3 consecutive \'X\'s in last component.\nIf TEMPLATE is not specified, use tmp.XXXXXXXXXX, and --tmpdir is implied.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Files are created u+rw, and directories u+rwx, minus umask restrictions.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    fputc_unlocked(10,_stdout);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -d, --directory     create a directory, not a file\n  -u, --dry-run       do not create anything; merely print a name (unsafe)\n  -q, --quiet         suppress diagnostics about file/dir-creation failure\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --suffix=SUFF   append SUFF to TEMPLATE; SUFF must not contain a slash.\n                        This option is implied if TEMPLATE does not end in X\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -p DIR, --tmpdir[=DIR]  interpret TEMPLATE relative to DIR; if DIR is not\n                        specified, use $TMPDIR if set, else /tmp.  With\n                        this option, TEMPLATE must not be an absolute name;\n                        unlike with -t, TEMPLATE may contain slashes, but\n                        mktemp creates only the final component\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -t                  interpret TEMPLATE as a single file name component,\n                        relative to a directory: $TMPDIR, if set; else the\n                        directory specified via -p; else /tmp [deprecated]\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info("mktemp");
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: count_consecutive_X_s
long count_consecutive_X_s(long param_1,long param_2) {
  long local_28;
  long local_10;
  
  local_10 = 0;
  for (local_28 = param_2; (local_28 != 0 && (*(char *)(param_1 + local_28 + -1) == 'X'));
      local_28 = local_28 + -1) {
    local_10 = local_10 + 1;
  }
  return local_10;
}

// Function: mkstemp_len
void mkstemp_len(undefined8 param_1,undefined4 param_2,undefined8 param_3,char param_4) {
  undefined8 uVar1;
  
  if (param_4 == '\0') {
    uVar1 = 0;
  }
  else {
    uVar1 = 2;
  }
  gen_tempname_len(param_1,param_2,0,uVar1,param_3);
  return;
}

// Function: mkdtemp_len
void mkdtemp_len(undefined8 param_1,undefined4 param_2,undefined8 param_3,char param_4) {
  undefined8 uVar1;
  
  if (param_4 == '\0') {
    uVar1 = 1;
  }
  else {
    uVar1 = 2;
  }
  gen_tempname_len(param_1,param_2,0,uVar1,param_3);
  return;
}

// Function: maybe_close_stdout
void maybe_close_stdout(void) {
  int iVar1;
  
  if (stdout_closed == '\x01') {
    iVar1 = close_stream(_stderr);
    if (iVar1 != 0) {
                    /* WARNING: Subroutine does not return */
      _exit(1);
    }
  }
  else {
    close_stdout();
  }
  return;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  bool bVar1;
  bool bVar2;
  bool bVar3;
  bool bVar4;
  int iVar5;
  int iVar6;
  undefined8 uVar7;
  size_t sVar8;
  undefined8 uVar9;
  char *pcVar10;
  ulong uVar11;
  int *piVar12;
  char local_111;
  int local_110;
  char *local_b0;
  char *local_a8;
  char *local_a0;
  char *local_98;
  size_t local_90;
  char *local_88;
  
  local_a8 = (char *)0x0;
  bVar4 = false;
  local_98 = (char *)0x0;
  bVar1 = false;
  bVar2 = false;
  bVar3 = false;
  local_111 = '\0';
  local_110 = 0;
  local_88 = (char *)0x0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit(maybe_close_stdout);
  while (iVar5 = getopt_long(param_1,param_2,"dp:qtuV",longopts,0), iVar6 = _optind, iVar5 != -1) {
    if (iVar5 < 0x81) {
      if (iVar5 < 0x56) {
        if (iVar5 == -0x83) {
switchD_00100673_caseD_56:
          uVar7 = proper_name_lite("Eric Blake","Eric Blake");
          uVar9 = proper_name_lite("Jim Meyering","Jim Meyering");
          version_etc(_stdout,"mktemp","GNU coreutils",_Version,uVar9,uVar7,0);
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        if (iVar5 == -0x82) {
          usage(0);
          goto switchD_00100673_caseD_56;
        }
        goto switchD_00100673_caseD_57;
      }
      switch(iVar5) {
      case 0x56:
        goto switchD_00100673_caseD_56;
      default:
        goto switchD_00100673_caseD_57;
      case 100:
        bVar3 = true;
        break;
      case 0x70:
        local_a8 = _optarg;
        bVar1 = true;
        break;
      case 0x71:
        bVar4 = true;
        break;
      case 0x74:
        bVar1 = true;
        bVar2 = true;
        break;
      case 0x75:
        local_111 = '\x01';
        break;
      case 0x80:
        local_98 = _optarg;
      }
    }
    else {
switchD_00100673_caseD_57:
      usage(1);
    }
  }
  if (1 < param_1 - _optind) {
    uVar7 = gettext("too many templates");
    error(0,0,uVar7);
    usage(1);
  }
  if (param_1 == iVar6) {
    bVar1 = true;
    local_a0 = default_template;
  }
  else {
    local_a0 = (char *)param_2[_optind];
  }
  if (local_98 == (char *)0x0) {
    local_a0 = (char *)xstrdup(local_a0);
    local_98 = strrchr(local_a0,0x58);
    if (local_98 == (char *)0x0) {
      sVar8 = strlen(local_a0);
      local_98 = local_a0 + sVar8;
    }
    else {
      local_98 = local_98 + 1;
    }
    local_90 = strlen(local_98);
  }
  else {
    sVar8 = strlen(local_a0);
    if ((sVar8 == 0) || (local_a0[sVar8 - 1] != 'X')) {
      uVar7 = quote(local_a0);
      uVar9 = gettext("with --suffix, template %s must end in X");
      error(1,0,uVar9,uVar7);
    }
    local_90 = strlen(local_98);
    local_88 = (char *)xcharalloc(local_90 + sVar8 + 1);
    memcpy(local_88,local_a0,sVar8);
    memcpy(local_88 + sVar8,local_98,local_90 + 1);
    local_98 = local_88 + sVar8;
    local_a0 = local_88;
  }
  if ((local_90 != 0) && (pcVar10 = (char *)last_component(local_98), local_98 != pcVar10)) {
    uVar7 = quote(local_98);
    uVar9 = gettext("invalid suffix %s, contains directory separator");
    error(1,0,uVar9,uVar7);
  }
  uVar11 = count_consecutive_X_s(local_a0,(long)local_98 - (long)local_a0);
  if (uVar11 < 3) {
    if (local_a0 == local_88) {
      *local_98 = '\0';
    }
    uVar7 = quote(local_a0);
    uVar9 = gettext("too few X\'s in template %s");
    error(1,0,uVar9,uVar7);
  }
  if (!bVar1) goto LAB_00100d1c;
  if (bVar2) {
    local_b0 = getenv("TMPDIR");
    if ((local_b0 == (char *)0x0) || (*local_b0 == '\0')) {
      if ((local_a8 == (char *)0x0) || (*local_a8 == '\0')) {
        local_b0 = "/tmp";
      }
      else {
        local_b0 = local_a8;
      }
    }
    pcVar10 = (char *)last_component(local_a0);
    if (local_a0 != pcVar10) {
      uVar7 = quote(local_a0);
      uVar9 = gettext("invalid template, %s, contains directory separator");
      error(1,0,uVar9,uVar7);
      goto LAB_00100c18;
    }
  }
  else {
LAB_00100c18:
    if ((local_a8 == (char *)0x0) || (*local_a8 == '\0')) {
      local_b0 = getenv("TMPDIR");
      if ((local_b0 == (char *)0x0) || (*local_b0 == '\0')) {
        local_b0 = "/tmp";
      }
    }
    else {
      local_b0 = local_a8;
    }
    if (*local_a0 == '/') {
      uVar7 = quote(local_a0);
      uVar9 = gettext("invalid template, %s; with --tmpdir, it may not be absolute");
      error(1,0,uVar9,uVar7);
    }
  }
  pcVar10 = (char *)file_name_concat(local_b0,local_a0,0);
  free(local_a0);
  local_a0 = pcVar10;
LAB_00100d1c:
  pcVar10 = (char *)xstrdup(local_a0);
  if (bVar3) {
    iVar6 = mkdtemp_len(pcVar10,local_90,uVar11,local_111);
    if (iVar6 != 0) {
      if (!bVar4) {
        uVar7 = quote(local_a0);
        uVar9 = gettext("failed to create directory via template %s");
        piVar12 = __errno_location();
        error(0,*piVar12,uVar9,uVar7);
      }
      local_110 = 1;
    }
  }
  else {
    iVar6 = mkstemp_len(pcVar10,local_90,uVar11,local_111);
    if ((iVar6 < 0) || ((local_111 != '\x01' && (iVar6 = close(iVar6), iVar6 != 0)))) {
      if (!bVar4) {
        uVar7 = quote(local_a0);
        uVar9 = gettext("failed to create file via template %s");
        piVar12 = __errno_location();
        error(0,*piVar12,uVar9,uVar7);
      }
      local_110 = 1;
    }
  }
  if ((local_110 == 0) && (puts(pcVar10), local_111 != '\x01')) {
    stdout_closed = 1;
    iVar6 = close_stream(_stdout);
    if (iVar6 != 0) {
      piVar12 = __errno_location();
      iVar6 = *piVar12;
      remove(pcVar10);
      if (!bVar4) {
        uVar7 = gettext("write error");
        error(0,iVar6,uVar7);
      }
      local_110 = 1;
    }
  }
                    /* WARNING: Subroutine does not return */
  exit(local_110);
}

