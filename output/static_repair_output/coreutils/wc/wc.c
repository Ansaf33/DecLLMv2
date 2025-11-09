// Function: avx2_supported
char avx2_supported(void) {
  char cVar1;
  undefined8 uVar2;
  
  uVar2 = gcc_feature_to_glibc_hwcap(&DAT_001031f4);
  cVar1 = hwcap_allowed(uVar2);
  if ((cVar1 == '\0') || ((_DAT_0010405c & 0x400) == 0)) {
    cVar1 = '\0';
  }
  else {
    cVar1 = '\x01';
  }
  if (debug != '\0') {
    if (cVar1 == '\0') {
      uVar2 = gettext("avx2 support not detected");
    }
    else {
      uVar2 = gettext("using avx2 hardware support");
    }
    error(0,0,uVar2);
  }
  return cVar1;
}

// Function: avx512_supported
char avx512_supported(void) {
  char cVar1;
  undefined8 uVar2;
  
  uVar2 = gcc_feature_to_glibc_hwcap("avx512f");
  cVar1 = hwcap_allowed(uVar2);
  if ((cVar1 != '\0') && ((_DAT_0010405c & 0x8000) != 0)) {
    uVar2 = gcc_feature_to_glibc_hwcap("avx512bw");
    cVar1 = hwcap_allowed(uVar2);
    if ((cVar1 != '\0') && ((_DAT_0010405c & 0x200000) != 0)) {
      cVar1 = '\x01';
      goto LAB_0010056a;
    }
  }
  cVar1 = '\0';
LAB_0010056a:
  if (debug != '\0') {
    if (cVar1 == '\0') {
      uVar2 = gettext("avx512 support not detected");
    }
    else {
      uVar2 = gettext("using avx512 hardware support");
    }
    error(0,0,uVar2);
  }
  return cVar1;
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
                            "Print newline, word, and byte counts for each FILE, and a total line if\nmore than one FILE is specified.  A word is a nonempty sequence of non white\nspace delimited by white space characters or by start or end of input.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_stdin_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nThe options below may be used to select which counts are printed, always in\nthe following order: newline, word, character, byte, maximum line length.\n  -c, --bytes            print the byte counts\n  -m, --chars            print the character counts\n  -l, --lines            print the newline counts\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --files0-from=F    read input from the files specified by\n                           NUL-terminated names in file F;\n                           If F is - then read names from standard input\n  -L, --max-line-length  print the maximum display width\n  -w, --words            print the word counts\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --total=WHEN       when to print a line with total counts;\n                           WHEN can be: auto, always, only, never\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_001037e1);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: maybe_c32isnbspace
undefined8 maybe_c32isnbspace(undefined4 param_1) {
  int iVar1;
  
  if ((posixly_correct != '\x01') && (iVar1 = c32isnbspace(param_1), iVar1 != 0)) {
    return 1;
  }
  return 0;
}

// Function: write_counts
void write_counts(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
                 undefined8 param_5,char *param_6) {
  undefined8 uVar1;
  char *pcVar2;
  long in_FS_OFFSET;
  undefined5 *local_30;
  undefined local_28 [24];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_30 = (undefined5 *)((long)&format_sp_int_3 + 1);
  if (print_lines != '\0') {
    uVar1 = umaxtostr(param_1,local_28);
    printf((char *)((long)&format_sp_int_3 + 1),(ulong)number_width,uVar1);
    local_30 = &format_sp_int_3;
  }
  if (print_words != '\0') {
    uVar1 = umaxtostr(param_2,local_28);
    printf((char *)local_30,(ulong)number_width,uVar1);
    local_30 = &format_sp_int_3;
  }
  if (print_chars != '\0') {
    uVar1 = umaxtostr(param_3,local_28);
    printf((char *)local_30,(ulong)number_width,uVar1);
    local_30 = &format_sp_int_3;
  }
  if (print_bytes != '\0') {
    uVar1 = umaxtostr(param_4,local_28);
    printf((char *)local_30,(ulong)number_width,uVar1);
    local_30 = &format_sp_int_3;
  }
  if (print_linelength != '\0') {
    uVar1 = imaxtostr(param_5,local_28);
    printf((char *)local_30,(ulong)number_width,uVar1);
  }
  if (param_6 != (char *)0x0) {
    pcVar2 = strchr(param_6,10);
    if (pcVar2 != (char *)0x0) {
      param_6 = (char *)quotearg_n_style_colon(0,3,param_6);
    }
    printf(" %s",param_6);
  }
  putchar_unlocked(10);
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: wc_lines
int * wc_lines(int *param_1,int param_2) {
  long lVar1;
  bool bVar2;
  undefined *puVar3;
  char cVar4;
  int iVar5;
  ssize_t sVar6;
  int *piVar7;
  char *pcVar8;
  undefined *puVar9;
  long in_FS_OFFSET;
  long lStack_40070;
  long lStack_40068;
  long lStack_40060;
  char *pcStack_40058;
  char *pcStack_40050;
  char acStack_40018 [16];
  undefined auStack_40008 [262144];
  
  puVar3 = &stack0xfffffffffffffff8;
  do {
    puVar9 = puVar3;
    *(undefined8 *)(puVar9 + -0x1000) = *(undefined8 *)(puVar9 + -0x1000);
    puVar3 = puVar9 + -0x1000;
  } while (puVar9 + -0x1000 != auStack_40008);
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  if (use_avx512_2 == '\0') {
    *(undefined8 *)(puVar9 + -0x1088) = 0x1009be;
    cVar4 = avx512_supported();
    if (cVar4 == '\0') {
      use_avx512_2 = -1;
    }
    else {
      use_avx512_2 = '\x01';
    }
  }
  if (use_avx512_2 < '\x01') {
    if (use_avx2_1 == '\0') {
      *(undefined8 *)(puVar9 + -0x1088) = 0x100a0b;
      cVar4 = avx2_supported();
      if (cVar4 == '\0') {
        use_avx2_1 = -1;
      }
      else {
        use_avx2_1 = '\x01';
      }
    }
    if (use_avx2_1 < '\x01') {
      lStack_40070 = 0;
      lStack_40068 = 0;
      bVar2 = false;
      while( true ) {
        *(undefined8 *)(puVar9 + -0x1088) = 0x100a81;
        sVar6 = read(param_2,acStack_40018,0x40000);
        if (sVar6 < 1) break;
        lStack_40068 = lStack_40068 + sVar6;
        pcVar8 = acStack_40018 + sVar6;
        lStack_40060 = 0;
        if (bVar2) {
          *pcVar8 = '\n';
          pcStack_40050 = acStack_40018;
          while( true ) {
            *(undefined8 *)(puVar9 + -0x1088) = 0x100b9c;
            pcStack_40050 = (char *)rawmemchr(pcStack_40050,10);
            if (pcVar8 <= pcStack_40050) break;
            lStack_40060 = lStack_40060 + 1;
            pcStack_40050 = pcStack_40050 + 1;
          }
        }
        else {
          for (pcStack_40058 = acStack_40018; pcStack_40058 < pcVar8;
              pcStack_40058 = pcStack_40058 + 1) {
            lStack_40060 = lStack_40060 + (ulong)(*pcStack_40058 == '\n');
          }
        }
        bVar2 = lStack_40060 * 0xf <= sVar6;
        lStack_40070 = lStack_40070 + lStack_40060;
      }
      if (sVar6 == 0) {
        iVar5 = 0;
      }
      else {
        *(undefined8 *)(puVar9 + -0x1088) = 0x100aa1;
        piVar7 = __errno_location();
        iVar5 = *piVar7;
      }
      *param_1 = iVar5;
      *(long *)(param_1 + 2) = lStack_40070;
      *(long *)(param_1 + 4) = lStack_40068;
    }
    else {
      *(undefined8 *)(puVar9 + -0x1088) = 0x100a43;
      wc_lines_avx2(param_1,param_2);
    }
  }
  else {
    *(undefined8 *)(puVar9 + -0x1088) = 0x1009f6;
    wc_lines_avx512(param_1,param_2);
  }
  if (lVar1 == *(long *)(in_FS_OFFSET + 0x28)) {
    return param_1;
  }
                    /* WARNING: Subroutine does not return */
  *(undefined8 *)(puVar9 + -0x1088) = 0x100bfb;
  __stack_chk_fail();
}

// Function: wc
bool wc(int param_1,long param_2,int *param_3) {
  byte bVar1;
  long lVar2;
  undefined *puVar3;
  char cVar4;
  int iVar5;
  long lVar6;
  size_t sVar7;
  __off_t _Var8;
  __off_t _Var9;
  uint *puVar10;
  ssize_t sVar11;
  byte *pbVar12;
  undefined8 uVar13;
  byte *pbVar14;
  long lVar15;
  undefined *puVar16;
  long lVar17;
  long in_FS_OFFSET;
  bool bVar18;
  char cStack_40103;
  byte bStack_40102;
  bool bStack_40100;
  bool bStack_400ff;
  bool bStack_400fd;
  bool bStack_400fc;
  uint uStack_400f8;
  long lStack_400e8;
  long lStack_400e0;
  long lStack_400d8;
  long lStack_400d0;
  long lStack_400c8;
  long lStack_400c0;
  size_t sStack_400b8;
  byte *pbStack_400b0;
  ulong uStack_400a8;
  long lStack_400a0;
  long lStack_40098;
  byte *pbStack_40090;
  uint auStack_40038 [2];
  long lStack_40030;
  long lStack_40028;
  mbstate_t mStack_40020;
  byte abStack_40018 [16];
  undefined auStack_40008 [262144];
  
  puVar3 = &stack0xfffffffffffffff8;
  do {
    puVar16 = puVar3;
    *(undefined8 *)(puVar16 + -0x1000) = *(undefined8 *)(puVar16 + -0x1000);
    puVar3 = puVar16 + -0x1000;
  } while (puVar16 + -0x1000 != auStack_40008);
  lVar2 = *(long *)(in_FS_OFFSET + 0x28);
  uStack_400f8 = 0;
  lVar6 = param_2;
  if (param_2 == 0) {
    *(undefined8 *)(puVar16 + -0x1128) = 0x100c72;
    lVar6 = gettext("standard input");
  }
  lStack_400c8 = 0;
  lStack_400d0 = 0;
  lStack_400d8 = 0;
  lStack_400e0 = 0;
  lStack_400e8 = 0;
  *(undefined8 *)(puVar16 + -0x1128) = 0x100cca;
  sVar7 = __ctype_get_mb_cur_max();
  if (sVar7 < 2) {
    if ((print_bytes == '\0') && (print_chars == 0)) {
      cStack_40103 = '\0';
    }
    else {
      cStack_40103 = '\x01';
    }
    bStack_40102 = 0;
  }
  else {
    cStack_40103 = print_bytes;
    bStack_40102 = print_chars;
  }
  if ((print_words == '\0') && (print_linelength == '\0')) {
    bVar18 = false;
  }
  else {
    bVar18 = true;
  }
  if ((((cStack_40103 != '\x01') || (bStack_40102 != 0)) || (print_lines != '\0')) || (bVar18)) {
    *(undefined8 *)(puVar16 + -0x1128) = 0x100d98;
    fdadvise(param_1,0,0,2);
  }
  if (((cStack_40103 == '\0') || (bStack_40102 == 1)) || ((print_lines == '\x01' || (bVar18)))) {
    if ((bStack_40102 == 1) || (bVar18)) {
      *(undefined8 *)(puVar16 + -0x1128) = 0x1010ca;
      sVar7 = __ctype_get_mb_cur_max();
      if (sVar7 < 2) {
        bStack_400fc = false;
        lStack_400a0 = 0;
        while( true ) {
          *(undefined8 *)(puVar16 + -0x1128) = 0x10174a;
          lStack_40098 = read(param_1,abStack_40018,0x40000);
          if (lStack_40098 == 0) break;
          if (lStack_40098 < 0) {
            *(undefined8 *)(puVar16 + -0x1128) = 0x1015a0;
            puVar10 = (uint *)__errno_location();
            uStack_400f8 = *puVar10;
            break;
          }
          lStack_400d0 = lStack_400d0 + lStack_40098;
          pbStack_40090 = abStack_40018;
          do {
            bVar1 = *pbStack_40090;
            switch(bVar1) {
            case 9:
              lStack_400a0 = lStack_400a0 + (8 - lStack_400a0 % 8);
              bStack_400fc = false;
              break;
            case 10:
              lStack_400e8 = lStack_400e8 + 1;
            case 0xc:
            case 0xd:
              if (lStack_400c8 < lStack_400a0) {
                lStack_400c8 = lStack_400a0;
              }
              lStack_400a0 = 0;
              bStack_400fc = false;
              break;
            default:
              lStack_400a0 = lStack_400a0 + (ulong)(byte)wc_isprint[(int)(uint)bVar1];
              lStack_400e0 = lStack_400e0 +
                             (int)(uint)(wc_isspace[(int)(uint)bVar1] == '\0' & (bStack_400fc ^ 1U))
              ;
              bStack_400fc = wc_isspace[(int)(uint)bVar1] == '\0';
              break;
            case 0x20:
              lStack_400a0 = lStack_400a0 + 1;
            case 0xb:
              bStack_400fc = false;
            }
            lStack_40098 = lStack_40098 + -1;
            pbStack_40090 = pbStack_40090 + 1;
          } while (lStack_40098 != 0);
        }
        if (lStack_400c8 < lStack_400a0) {
          lStack_400c8 = lStack_400a0;
        }
      }
      else {
        bStack_40100 = false;
        lStack_400c0 = 0;
        *(undefined8 *)(puVar16 + -0x1128) = 0x1010f5;
        mbszero(&mStack_40020);
        bStack_400ff = false;
        sStack_400b8 = 0;
LAB_001014fc:
        *(undefined8 *)(puVar16 + -0x1128) = 0x101530;
        sVar11 = read(param_1,abStack_40018 + sStack_400b8,0x40000 - sStack_400b8);
        if ((sVar11 == 0) && (sStack_400b8 == 0)) goto LAB_00101553;
        if (-1 < sVar11) {
          lStack_400d0 = lStack_400d0 + sVar11;
          pbStack_400b0 = abStack_40018;
          pbVar12 = pbStack_400b0 + sStack_400b8 + sVar11;
          do {
            if ((bStack_400ff == true) || ((char)*pbStack_400b0 < '\0')) {
              pbVar14 = pbStack_400b0 + sStack_400b8;
              *(undefined8 *)(puVar16 + -0x1128) = 0x1011fd;
              uStack_400a8 = rpl_mbrtoc32(auStack_40038,pbStack_400b0 + sStack_400b8,
                                          (long)pbVar12 - (long)pbVar14,&mStack_40020);
              sStack_400b8 = 0;
              if (uStack_400a8 <= (ulong)((long)pbVar12 - (long)pbVar14)) {
                uStack_400a8 = uStack_400a8 + (uStack_400a8 == 0);
                bVar18 = uStack_400a8 == (bStack_400ff ^ 1);
                *(undefined8 *)(puVar16 + -0x1128) = 0x101317;
                iVar5 = mbsinit(&mStack_40020);
                bStack_400ff = iVar5 == 0;
                goto LAB_00101322;
              }
              if (((uStack_400a8 == 0xfffffffffffffffe) &&
                  ((long)pbVar12 - (long)pbStack_400b0 < 0x40000)) && (sVar11 != 0)) {
                sStack_400b8 = (long)pbVar12 - (long)pbStack_400b0;
                *(undefined8 *)(puVar16 + -0x1128) = 0x101282;
                memmove(abStack_40018,pbStack_400b0,sStack_400b8);
                bStack_400ff = true;
                break;
              }
              pbStack_400b0 = pbStack_400b0 + 1;
              *(undefined8 *)(puVar16 + -0x1128) = 0x1012a5;
              mbszero(&mStack_40020);
              bStack_400ff = false;
              lStack_400e0 = lStack_400e0 + (ulong)(bStack_40100 ^ 1);
              bStack_40100 = true;
            }
            else {
              uStack_400a8 = 1;
              auStack_40038[0] = (uint)(char)*pbStack_400b0;
              bVar18 = true;
LAB_00101322:
              switch(auStack_40038[0]) {
              case 9:
                lStack_400c0 = lStack_400c0 + (8 - lStack_400c0 % 8);
                bStack_40100 = false;
                break;
              case 10:
                lStack_400e8 = lStack_400e8 + 1;
              case 0xc:
              case 0xd:
                if (lStack_400c8 < lStack_400c0) {
                  lStack_400c8 = lStack_400c0;
                }
                lStack_400c0 = 0;
                bStack_40100 = false;
                break;
              default:
                if (bVar18) {
                  lStack_400c0 = lStack_400c0 + (ulong)(byte)wc_isprint[auStack_40038[0]];
                  bStack_400fd = wc_isspace[auStack_40038[0]] == '\0';
                }
                else {
                  if (print_linelength != '\0') {
                    *(undefined8 *)(puVar16 + -0x1128) = 0x10144c;
                    iVar5 = c32width(auStack_40038[0]);
                    if (0 < iVar5) {
                      lStack_400c0 = lStack_400c0 + iVar5;
                    }
                  }
                  *(undefined8 *)(puVar16 + -0x1128) = 0x101477;
                  iVar5 = c32isspace(auStack_40038[0]);
                  if (iVar5 == 0) {
                    *(undefined8 *)(puVar16 + -0x1128) = 0x101488;
                    iVar5 = maybe_c32isnbspace(auStack_40038[0]);
                    if (iVar5 == 0) {
                      bStack_400fd = true;
                      goto LAB_001014a5;
                    }
                  }
                  bStack_400fd = false;
                }
LAB_001014a5:
                lStack_400e0 = lStack_400e0 + (int)(uint)(bStack_400fd & (bStack_40100 ^ 1U));
                bStack_40100 = bStack_400fd;
                break;
              case 0x20:
                lStack_400c0 = lStack_400c0 + 1;
              case 0xb:
                bStack_40100 = false;
              }
              pbStack_400b0 = pbStack_400b0 + uStack_400a8;
              lStack_400d8 = lStack_400d8 + 1;
            }
          } while (pbStack_400b0 < pbVar12);
          goto LAB_001014fc;
        }
        *(undefined8 *)(puVar16 + -0x1128) = 0x10111b;
        puVar10 = (uint *)__errno_location();
        uStack_400f8 = *puVar10;
LAB_00101553:
        if (lStack_400c8 < lStack_400c0) {
          lStack_400c8 = lStack_400c0;
        }
      }
    }
    else {
      *(undefined8 *)(puVar16 + -0x1128) = 0x101098;
      wc_lines(auStack_40038,param_1);
      uStack_400f8 = auStack_40038[0];
      lStack_400e8 = lStack_40030;
      lStack_400d0 = lStack_40028;
    }
    goto LAB_00101780;
  }
  bVar18 = false;
  if (0 < *param_3) {
    *(undefined8 *)(puVar16 + -0x1128) = 0x100e0a;
    iVar5 = fstat(param_1,(stat *)(param_3 + 2));
    *param_3 = iVar5;
  }
  if (*param_3 == 0) {
    *(undefined8 *)(puVar16 + -0x1128) = 0x100e37;
    cVar4 = usable_st_size(param_3 + 2);
    if ((cVar4 != '\0') && (-1 < *(long *)(param_3 + 0xe))) {
      lVar15 = *(long *)(param_3 + 0xe);
      *(undefined8 *)(puVar16 + -0x1128) = 0x100e7c;
      _Var8 = lseek(param_1,0,1);
      if (-1 < _Var8) {
        if (lVar15 % page_size == 0) {
          if ((*(long *)(param_3 + 0x10) < 1) || (0x2000000000000000 < *(ulong *)(param_3 + 0x10)))
          {
            lVar17 = 0x201;
          }
          else {
            lVar17 = *(long *)(param_3 + 0x10) + 1;
          }
          lVar15 = lVar15 - lVar15 % lVar17;
          if ((-1 < _Var8) && (_Var8 < lVar15)) {
            *(undefined8 *)(puVar16 + -0x1128) = 0x100fbe;
            _Var9 = lseek(param_1,lVar15,1);
            if (-1 < _Var9) {
              lStack_400d0 = lVar15 - _Var8;
            }
          }
        }
        else {
          if (lVar15 < _Var8) {
            lStack_400d0 = 0;
          }
          else {
            lStack_400d0 = lVar15 - _Var8;
          }
          if (lStack_400d0 != 0) {
            *(undefined8 *)(puVar16 + -0x1128) = 0x100f01;
            _Var8 = lseek(param_1,lStack_400d0,1);
            if (-1 < _Var8) {
              bVar18 = true;
              goto LAB_00100fd8;
            }
          }
          lStack_400d0 = 0;
        }
      }
    }
  }
LAB_00100fd8:
  if (!bVar18) {
    *(undefined8 *)(puVar16 + -0x1128) = 0x101006;
    fdadvise(param_1,0,0,2);
    while( true ) {
      *(undefined8 *)(puVar16 + -0x1128) = 0x10104f;
      sVar11 = read(param_1,abStack_40018,0x40000);
      if (sVar11 == 0) break;
      if (sVar11 < 0) {
        *(undefined8 *)(puVar16 + -0x1128) = 0x101017;
        puVar10 = (uint *)__errno_location();
        uStack_400f8 = *puVar10;
        break;
      }
      lStack_400d0 = lStack_400d0 + sVar11;
    }
  }
LAB_00101780:
  if (bStack_40102 < print_chars) {
    lStack_400d8 = lStack_400d0;
  }
  if (total_mode != 2) {
    *(undefined8 *)(puVar16 + -0x1128) = 0x1017e0;
    write_counts(lStack_400e8,lStack_400e0,lStack_400d8,lStack_400d0,lStack_400c8,param_2);
  }
  lVar15 = total_lines + -0x8000000000000000;
  total_lines = lVar15 + lStack_400e8 + -0x8000000000000000;
  total_lines_overflow = (byte)(SCARRY8(lVar15,lStack_400e8) | total_lines_overflow) != 0;
  lVar15 = total_words + -0x8000000000000000;
  total_words = lVar15 + lStack_400e0 + -0x8000000000000000;
  total_words_overflow = (byte)(SCARRY8(lVar15,lStack_400e0) | total_words_overflow) != 0;
  lVar15 = total_chars + -0x8000000000000000;
  total_chars = lVar15 + lStack_400d8 + -0x8000000000000000;
  total_chars_overflow = (byte)(SCARRY8(lVar15,lStack_400d8) | total_chars_overflow) != 0;
  lVar15 = total_bytes + -0x8000000000000000;
  total_bytes = lVar15 + lStack_400d0 + -0x8000000000000000;
  total_bytes_overflow = (byte)(SCARRY8(lVar15,lStack_400d0) | total_bytes_overflow) != 0;
  if (max_line_length < lStack_400c8) {
    max_line_length = lStack_400c8;
  }
  if (uStack_400f8 != 0) {
    *(undefined8 *)(puVar16 + -0x1128) = 0x10198c;
    uVar13 = quotearg_n_style_colon(0,3,lVar6);
    *(undefined8 *)(puVar16 + -0x1128) = 0x1019d2;
    error(0,uStack_400f8,&DAT_001037f7,uVar13);
  }
  if (lVar2 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    *(undefined8 *)(puVar16 + -0x1128) = 0x1019f1;
    __stack_chk_fail();
  }
  return uStack_400f8 == 0;
}

// Function: wc_file
ulong wc_file(char *param_1,undefined8 param_2) {
  char cVar1;
  byte bVar2;
  int iVar3;
  ulong uVar4;
  undefined8 uVar5;
  int *piVar6;
  
  if ((param_1 == (char *)0x0) || (cVar1 = streq(param_1,&DAT_001038bc), cVar1 != '\0')) {
    have_read_stdin = 1;
    xset_binary_mode(0,0);
    uVar4 = wc(0,param_1,param_2);
    return uVar4;
  }
  iVar3 = open(param_1,0);
  if (iVar3 == -1) {
    uVar5 = quotearg_n_style_colon(0,3,param_1);
    piVar6 = __errno_location();
    error(0,*piVar6,&DAT_001037f7,uVar5);
    return 0;
  }
  bVar2 = wc(iVar3,param_1,param_2);
  iVar3 = close(iVar3);
  if (iVar3 != 0) {
    uVar5 = quotearg_n_style_colon(0,3,param_1);
    piVar6 = __errno_location();
    error(0,*piVar6,&DAT_001037f7,uVar5);
    return 0;
  }
  return (ulong)bVar2;
}

// Function: get_input_fstatus
undefined4 * get_input_fstatus(long param_1,long param_2) {
  char cVar1;
  int iVar2;
  long lVar3;
  undefined4 *puVar4;
  long local_18;
  
  lVar3 = param_1;
  if (param_1 == 0) {
    lVar3 = 1;
  }
  puVar4 = (undefined4 *)xnmalloc(lVar3,0x98);
  if ((param_1 == 0) ||
     ((param_1 == 1 &&
      ((uint)print_linelength +
       (uint)print_lines + (uint)print_words + (uint)print_chars + (uint)print_bytes == 1)))) {
    *puVar4 = 1;
  }
  else {
    for (local_18 = 0; local_18 < param_1; local_18 = local_18 + 1) {
      if ((*(long *)(param_2 + local_18 * 8) == 0) ||
         (cVar1 = streq(*(undefined8 *)(param_2 + local_18 * 8),&DAT_001038bc), cVar1 != '\0')) {
        iVar2 = fstat(0,(stat *)(puVar4 + local_18 * 0x26 + 2));
      }
      else {
        iVar2 = stat(*(char **)(param_2 + local_18 * 8),(stat *)(puVar4 + local_18 * 0x26 + 2));
      }
      puVar4[local_18 * 0x26] = iVar2;
    }
  }
  return puVar4;
}

// Function: compute_number_width
int compute_number_width(long param_1,int *param_2) {
  long lVar1;
  long in_FS_OFFSET;
  int local_28;
  int local_24;
  ulong local_20;
  long local_18;
  
  local_28 = 1;
  if ((0 < param_1) && (*param_2 < 1)) {
    local_24 = 1;
    local_20 = 0;
    for (local_18 = 0; local_18 < param_1; local_18 = local_18 + 1) {
      if (param_2[local_18 * 0x26] == 0) {
        if ((param_2[local_18 * 0x26 + 8] & 0xf000U) == 0x8000) {
          lVar1 = local_20 + 0x8000000000000000;
          local_20 = lVar1 + *(long *)(param_2 + local_18 * 0x26 + 0xe) + 0x8000000000000000;
          if (SCARRY8(lVar1,*(long *)(param_2 + local_18 * 0x26 + 0xe))) {
            local_20 = 0xffffffffffffffff;
            break;
          }
        }
        else {
          local_24 = 7;
        }
      }
    }
    for (; 9 < local_20; local_20 = local_20 / 10) {
      local_28 = local_28 + 1;
    }
    if (local_28 < local_24) {
      local_28 = local_24;
    }
  }
  if (*(long *)(in_FS_OFFSET + 0x28) != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_28;
}

// Function: main
byte main(int param_1,undefined8 *param_2) {
  bool bVar1;
  bool bVar2;
  undefined uVar3;
  char cVar4;
  byte bVar5;
  int iVar6;
  undefined4 uVar7;
  char *pcVar8;
  ushort **ppuVar9;
  undefined8 uVar10;
  undefined8 uVar11;
  undefined8 uVar12;
  long lVar13;
  ulong uVar14;
  int *piVar15;
  long in_FS_OFFSET;
  double dVar16;
  double dVar17;
  bool local_2e6;
  uint local_2e4;
  int local_2e0;
  int local_2dc;
  int local_2d8;
  int local_2d4;
  undefined4 local_2d0;
  undefined4 local_2cc;
  undefined4 local_2c8;
  int local_2c4;
  undefined4 local_2c0;
  undefined4 local_2bc;
  undefined4 local_2b8;
  undefined4 local_2b4;
  undefined4 local_2b0;
  undefined4 local_2ac;
  undefined4 local_2a8;
  undefined4 local_2a4;
  undefined4 local_2a0;
  int local_29c;
  undefined4 local_298;
  undefined4 local_294;
  undefined4 local_290;
  undefined4 local_28c;
  undefined4 local_288;
  undefined4 local_284;
  undefined4 local_280;
  undefined4 local_27c;
  undefined4 local_278;
  int local_274;
  long local_270;
  undefined8 *local_268;
  long local_260;
  long local_258;
  FILE *local_250;
  undefined4 *local_248;
  char *local_240;
  undefined8 local_238;
  undefined8 local_230;
  undefined8 local_228;
  undefined8 local_220;
  undefined *local_218;
  char *local_210;
  undefined8 local_208;
  undefined8 local_200;
  undefined8 local_1f8;
  undefined8 local_1f0;
  undefined8 local_1e8;
  undefined *local_1e0;
  long local_1d8;
  undefined8 *local_1d0;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_260 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  iVar6 = getpagesize();
  page_size = (long)iVar6;
  setvbuf(_stdout,(char *)0x0,1,0);
  pcVar8 = getenv("POSIXLY_CORRECT");
  posixly_correct = pcVar8 != (char *)0x0;
  print_bytes = '\0';
  print_chars = '\0';
  print_words = '\0';
  print_lines = '\0';
  print_linelength = '\0';
  max_line_length = 0;
  total_bytes = 0;
  total_chars = 0;
  total_words = 0;
  total_lines = 0;
  while (local_2d4 = getopt_long(param_1,param_2,"clLmw",longopts,0), local_2d4 != -1) {
    if (local_2d4 < 0x83) {
      if (local_2d4 < 0x4c) {
        if (local_2d4 == -0x83) {
LAB_00102121:
          uVar10 = proper_name_lite("David MacKenzie","David MacKenzie");
          uVar12 = proper_name_lite("Paul Rubin","Paul Rubin");
          version_etc(_stdout,&DAT_001037e1,"GNU coreutils",_Version,uVar12,uVar10,0);
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        if (local_2d4 == -0x82) {
          usage(0);
          goto LAB_00102121;
        }
        goto switchD_00102058_caseD_4d;
      }
      switch(local_2d4) {
      case 0x4c:
        print_linelength = '\x01';
        break;
      default:
        goto switchD_00102058_caseD_4d;
      case 99:
        print_bytes = '\x01';
        break;
      case 0x6c:
        print_lines = '\x01';
        break;
      case 0x6d:
        print_chars = '\x01';
        break;
      case 0x77:
        print_words = '\x01';
        break;
      case 0x80:
        debug = 1;
        break;
      case 0x81:
        local_260 = _optarg;
        break;
      case 0x82:
        lVar13 = __xargmatch_internal("--total",_optarg,total_args,total_types,4,_argmatch_die,1);
        total_mode = *(int *)(total_types + lVar13 * 4);
      }
    }
    else {
switchD_00102058_caseD_4d:
      usage(1);
    }
  }
  if ((((print_lines != '\x01') && (print_words != '\x01')) && (print_chars != '\x01')) &&
     ((print_bytes != '\x01' && (print_linelength != '\x01')))) {
    print_bytes = '\x01';
    print_words = '\x01';
    print_lines = '\x01';
  }
  local_2d4 = 0xffffffff;
  if (print_linelength != '\0') {
    for (local_2e0 = 0; local_2e0 < 0x100; local_2e0 = local_2e0 + 1) {
      ppuVar9 = __ctype_b_loc();
      wc_isprint[local_2e0] = ((*ppuVar9)[local_2e0] & 0x4000) != 0;
    }
  }
  if (print_words != '\0') {
    for (local_2dc = 0; local_2dc < 0x100; local_2dc = local_2dc + 1) {
      ppuVar9 = __ctype_b_loc();
      if (((*ppuVar9)[local_2dc] & 0x2000) == 0) {
        uVar7 = btoc32(local_2dc);
        iVar6 = maybe_c32isnbspace(uVar7);
        if (iVar6 != 0) goto LAB_00102306;
        uVar3 = 0;
      }
      else {
LAB_00102306:
        uVar3 = 1;
      }
      wc_isspace[local_2dc] = uVar3;
    }
  }
  bVar2 = false;
  if (local_260 == 0) {
    if (_optind < param_1) {
      local_268 = param_2 + _optind;
      local_270 = (long)(param_1 - _optind);
    }
    else {
      local_268 = &stdin_only_0;
      local_270 = 1;
    }
    local_258 = argv_iter_init_argv(local_268);
  }
  else {
    if (_optind < param_1) {
      uVar10 = quotearg_style(4,param_2[_optind]);
      local_238 = gettext("extra operand %s");
      local_2d0 = 0;
      local_2cc = 0;
      error(0,0,local_238,uVar10);
      uVar10 = gettext("file operands cannot be combined with --files0-from");
      fprintf(_stderr,"%s\n",uVar10);
      usage(1);
    }
    cVar4 = streq(local_260,&DAT_001038bc);
    if (cVar4 == '\0') {
      local_250 = (FILE *)rpl_fopen(local_260,&DAT_00103968);
      if (local_250 == (FILE *)0x0) {
        uVar10 = quotearg_style(4,local_260);
        uVar12 = gettext("cannot open %s for reading");
        piVar15 = __errno_location();
        local_2c4 = *piVar15;
        local_2c8 = 1;
        local_230 = uVar12;
        error(1,local_2c4,uVar12,uVar10);
      }
    }
    else {
      local_250 = _stdin;
    }
    iVar6 = fileno_unlocked(local_250);
    iVar6 = fstat(iVar6,&local_b8);
    if ((iVar6 == 0) && ((local_b8.st_mode & 0xf000) == 0x8000)) {
      dVar16 = (double)physmem_available();
      dVar17 = DAT_00103b50;
      if (dVar16 / DAT_00103b48 <= DAT_00103b50) {
        dVar17 = (double)physmem_available();
        dVar17 = dVar17 / DAT_00103b48;
      }
      if ((double)local_b8.st_size <= dVar17) {
        bVar2 = true;
        readtokens0_init(&local_1d8);
        cVar4 = readtokens0(local_250,&local_1d8);
        if ((cVar4 != '\x01') || (iVar6 = rpl_fclose(local_250), iVar6 != 0)) {
          uVar10 = quotearg_style(4,local_260);
          local_228 = gettext("cannot read file names from %s");
          local_2c0 = 1;
          local_2bc = 0;
          error(1,0,local_228,uVar10);
        }
        local_268 = local_1d0;
        local_270 = local_1d8;
        local_258 = argv_iter_init_argv(local_1d0);
        goto LAB_00102714;
      }
    }
    local_268 = (undefined8 *)0x0;
    local_270 = 0;
    local_258 = argv_iter_init_stream(local_250);
  }
LAB_00102714:
  if (local_258 == 0) {
    xalloc_die();
  }
  local_248 = (undefined4 *)get_input_fstatus(local_270,local_268);
  if (total_mode == 2) {
    number_width = 1;
  }
  else {
    number_width = compute_number_width(local_270,local_248);
  }
  local_2e6 = true;
  local_2d8 = 0;
  while (local_240 = (char *)argv_iter(local_258,&local_2e4), local_240 != (char *)0x0) {
    bVar1 = false;
    if (((local_260 != 0) && (cVar4 = streq(local_260,&DAT_001038bc), cVar4 != '\0')) &&
       (cVar4 = streq(local_240,&DAT_001038bc), cVar4 != '\0')) {
      uVar10 = quotearg_style(4,local_240);
      local_220 = gettext("when reading file names from standard input, no file name of %s allowed")
      ;
      local_2b8 = 0;
      local_2b4 = 0;
      error(0,0,local_220,uVar10);
      bVar1 = true;
    }
    if (*local_240 == '\0') {
      if (local_260 == 0) {
        uVar10 = gettext("invalid zero-length file name");
        local_2b0 = 0;
        local_2ac = 0;
        local_218 = &DAT_001037f7;
        error(0,0,&DAT_001037f7,uVar10);
      }
      else {
        uVar10 = gettext("invalid zero-length file name");
        uVar12 = argv_iter_n_args(local_258);
        uVar11 = quotearg_n_style_colon(0,3,local_260);
        local_2a8 = 0;
        local_2a4 = 0;
        local_210 = "%s:%zu: %s";
        error(0,0,"%s:%zu: %s",uVar11,uVar12,uVar10);
      }
      bVar1 = true;
    }
    if (bVar1) {
      local_2e6 = false;
    }
    else {
      if (local_270 == 0) {
        lVar13 = 0;
      }
      else {
        lVar13 = (long)local_2d8 * 0x98;
      }
      bVar5 = wc_file(local_240,lVar13 + (long)local_248);
      local_2e6 = (bVar5 & local_2e6) != 0;
    }
    if (local_270 == 0) {
      *local_248 = 1;
    }
    local_2d8 = local_2d8 + 1;
  }
  if (local_2e4 == 4) {
    uVar10 = quotearg_n_style_colon(0,3,local_260);
    uVar12 = gettext("%s: read error");
    piVar15 = __errno_location();
    local_29c = *piVar15;
    local_2a0 = 0;
    local_208 = uVar12;
    error(0,local_29c,uVar12,uVar10);
    local_2e6 = false;
  }
  else if (((local_2e4 < 5) && (local_2e4 != 2)) && (local_2e4 == 3)) {
    xalloc_die();
  }
  if (((local_2e6 != false) && (local_260 == 0)) &&
     (lVar13 = argv_iter_n_args(local_258), lVar13 == 0)) {
    bVar5 = wc_file(0,local_248);
    local_2e6 = (bVar5 & local_2e6) != 0;
  }
  if (bVar2) {
    readtokens0_free(&local_1d8);
  }
  if ((total_mode != 3) && ((total_mode != 0 || (uVar14 = argv_iter_n_args(local_258), 1 < uVar14)))
     ) {
    if (total_lines_overflow != '\0') {
      total_lines = 0xffffffffffffffff;
      local_200 = gettext("total lines");
      local_298 = 0;
      local_294 = 0x4b;
      error(0,0x4b,local_200);
      local_2e6 = false;
    }
    if (total_words_overflow != '\0') {
      total_words = 0xffffffffffffffff;
      local_1f8 = gettext("total words");
      local_290 = 0;
      local_28c = 0x4b;
      error(0,0x4b,local_1f8);
      local_2e6 = false;
    }
    if (total_chars_overflow != '\0') {
      total_chars = 0xffffffffffffffff;
      local_1f0 = gettext("total characters");
      local_288 = 0;
      local_284 = 0x4b;
      error(0,0x4b,local_1f0);
      local_2e6 = false;
    }
    if (total_bytes_overflow != '\0') {
      total_bytes = 0xffffffffffffffff;
      local_1e8 = gettext("total bytes");
      local_280 = 0;
      local_27c = 0x4b;
      error(0,0x4b,local_1e8);
      local_2e6 = false;
    }
    if (total_mode == 2) {
      uVar10 = 0;
    }
    else {
      uVar10 = gettext("total");
    }
    write_counts(total_lines,total_words,total_chars,total_bytes,max_line_length,uVar10);
  }
  argv_iter_free(local_258);
  free(local_248);
  if ((have_read_stdin != '\0') && (iVar6 = close(0), iVar6 != 0)) {
    piVar15 = __errno_location();
    local_274 = *piVar15;
    local_278 = 1;
    local_1e0 = &DAT_001038bc;
    error(1,local_274,&DAT_001038bc);
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_2e6 ^ 1;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

