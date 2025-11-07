// Function: matcher_error
void matcher_error(void) {
  char cVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  int *piVar5;
  char *pcVar6;
  char *pcStack_48;
  int iStack_3c;
  char *pcStack_38;
  
  uVar4 = gettext("error in regular expression matcher");
  piVar5 = __errno_location();
  pcVar6 = (char *)0x1;
  error(1,*piVar5,uVar4);
  pcStack_38 = pcVar6;
  do {
    while( true ) {
      do {
        while( true ) {
          pcStack_48 = pcVar6;
          if (*pcStack_48 == '\0') {
            *pcStack_38 = '\0';
            return;
          }
          if (*pcStack_48 == '\\') break;
          *pcStack_38 = *pcStack_48;
          pcVar6 = pcStack_48 + 1;
          pcStack_38 = pcStack_38 + 1;
        }
        pcVar6 = pcStack_48 + 1;
        cVar2 = *pcVar6;
        iVar3 = (int)cVar2;
      } while (cVar2 == '\0');
      if (((-1 < cVar2) && (iVar3 < 0x79)) && (0x2f < iVar3)) break;
switchD_001003b3_caseD_31:
      *pcStack_38 = '\\';
      pcStack_38[1] = *pcVar6;
      pcVar6 = pcStack_48 + 2;
      pcStack_38 = pcStack_38 + 2;
    }
    switch(iVar3) {
    case 0x30:
      cVar2 = '\0';
      iStack_3c = 0;
      for (pcStack_48 = pcStack_48 + 2;
          ((iStack_3c < 3 && ('/' < *pcStack_48)) && (*pcStack_48 < '8'));
          pcStack_48 = pcStack_48 + 1) {
        cVar2 = *pcStack_48 + -0x30 + cVar2 * '\b';
        iStack_3c = iStack_3c + 1;
      }
      *pcStack_38 = cVar2;
      pcVar6 = pcStack_48;
      pcStack_38 = pcStack_38 + 1;
      break;
    default:
      goto switchD_001003b3_caseD_31;
    case 0x61:
      *pcStack_38 = '\a';
      pcVar6 = pcStack_48 + 2;
      pcStack_38 = pcStack_38 + 1;
      break;
    case 0x62:
      *pcStack_38 = '\b';
      pcVar6 = pcStack_48 + 2;
      pcStack_38 = pcStack_38 + 1;
      break;
    case 99:
      while (pcStack_48 = pcVar6, pcVar6 = pcStack_48, *pcStack_48 != '\0') {
        pcVar6 = pcStack_48 + 1;
      }
      break;
    case 0x66:
      *pcStack_38 = '\f';
      pcVar6 = pcStack_48 + 2;
      pcStack_38 = pcStack_38 + 1;
      break;
    case 0x6e:
      *pcStack_38 = '\n';
      pcVar6 = pcStack_48 + 2;
      pcStack_38 = pcStack_38 + 1;
      break;
    case 0x72:
      *pcStack_38 = '\r';
      pcVar6 = pcStack_48 + 2;
      pcStack_38 = pcStack_38 + 1;
      break;
    case 0x74:
      *pcStack_38 = '\t';
      pcVar6 = pcStack_48 + 2;
      pcStack_38 = pcStack_38 + 1;
      break;
    case 0x76:
      *pcStack_38 = '\v';
      pcVar6 = pcStack_48 + 2;
      pcStack_38 = pcStack_38 + 1;
      break;
    case 0x78:
      cVar2 = '\0';
      iStack_3c = 0;
      pcStack_48 = pcStack_48 + 2;
      while ((iStack_3c < 3 && (cVar1 = c_isxdigit((int)*pcStack_48), cVar1 != '\0'))) {
        if ((*pcStack_48 < 'a') || ('f' < *pcStack_48)) {
          if ((*pcStack_48 < 'A') || ('F' < *pcStack_48)) {
            cVar1 = *pcStack_48 + -0x30;
          }
          else {
            cVar1 = *pcStack_48 + -0x37;
          }
        }
        else {
          cVar1 = *pcStack_48 + -0x57;
        }
        cVar2 = cVar1 + cVar2 * '\x10';
        iStack_3c = iStack_3c + 1;
        pcStack_48 = pcStack_48 + 1;
      }
      pcVar6 = pcStack_48;
      if (iStack_3c == 0) {
        *pcStack_38 = '\\';
        pcStack_38[1] = 'x';
        pcStack_38 = pcStack_38 + 2;
      }
      else {
        *pcStack_38 = cVar2;
        pcStack_38 = pcStack_38 + 1;
      }
    }
  } while( true );
}

// Function: unescape_string
void unescape_string(char *param_1) {
  char *pcVar1;
  char cVar2;
  char cVar3;
  int iVar4;
  char *local_20;
  int local_14;
  char *local_10;
  
  pcVar1 = param_1;
  local_10 = param_1;
  do {
    while( true ) {
      do {
        while( true ) {
          local_20 = pcVar1;
          if (*local_20 == '\0') {
            *local_10 = '\0';
            return;
          }
          if (*local_20 == '\\') break;
          *local_10 = *local_20;
          pcVar1 = local_20 + 1;
          local_10 = local_10 + 1;
        }
        pcVar1 = local_20 + 1;
        cVar3 = *pcVar1;
        iVar4 = (int)cVar3;
      } while (cVar3 == '\0');
      if (((-1 < cVar3) && (iVar4 < 0x79)) && (0x2f < iVar4)) break;
switchD_001003b3_caseD_31:
      *local_10 = '\\';
      local_10[1] = *pcVar1;
      pcVar1 = local_20 + 2;
      local_10 = local_10 + 2;
    }
    switch(iVar4) {
    case 0x30:
      cVar3 = '\0';
      local_14 = 0;
      for (local_20 = local_20 + 2; ((local_14 < 3 && ('/' < *local_20)) && (*local_20 < '8'));
          local_20 = local_20 + 1) {
        cVar3 = *local_20 + -0x30 + cVar3 * '\b';
        local_14 = local_14 + 1;
      }
      *local_10 = cVar3;
      pcVar1 = local_20;
      local_10 = local_10 + 1;
      break;
    default:
      goto switchD_001003b3_caseD_31;
    case 0x61:
      *local_10 = '\a';
      pcVar1 = local_20 + 2;
      local_10 = local_10 + 1;
      break;
    case 0x62:
      *local_10 = '\b';
      pcVar1 = local_20 + 2;
      local_10 = local_10 + 1;
      break;
    case 99:
      while (local_20 = pcVar1, pcVar1 = local_20, *local_20 != '\0') {
        pcVar1 = local_20 + 1;
      }
      break;
    case 0x66:
      *local_10 = '\f';
      pcVar1 = local_20 + 2;
      local_10 = local_10 + 1;
      break;
    case 0x6e:
      *local_10 = '\n';
      pcVar1 = local_20 + 2;
      local_10 = local_10 + 1;
      break;
    case 0x72:
      *local_10 = '\r';
      pcVar1 = local_20 + 2;
      local_10 = local_10 + 1;
      break;
    case 0x74:
      *local_10 = '\t';
      pcVar1 = local_20 + 2;
      local_10 = local_10 + 1;
      break;
    case 0x76:
      *local_10 = '\v';
      pcVar1 = local_20 + 2;
      local_10 = local_10 + 1;
      break;
    case 0x78:
      cVar3 = '\0';
      local_14 = 0;
      local_20 = local_20 + 2;
      while ((local_14 < 3 && (cVar2 = c_isxdigit((int)*local_20), cVar2 != '\0'))) {
        if ((*local_20 < 'a') || ('f' < *local_20)) {
          if ((*local_20 < 'A') || ('F' < *local_20)) {
            cVar2 = *local_20 + -0x30;
          }
          else {
            cVar2 = *local_20 + -0x37;
          }
        }
        else {
          cVar2 = *local_20 + -0x57;
        }
        cVar3 = cVar2 + cVar3 * '\x10';
        local_14 = local_14 + 1;
        local_20 = local_20 + 1;
      }
      pcVar1 = local_20;
      if (local_14 == 0) {
        *local_10 = '\\';
        local_10[1] = 'x';
        local_10 = local_10 + 2;
      }
      else {
        *local_10 = cVar3;
        local_10 = local_10 + 1;
      }
    }
  } while( true );
}

// Function: compile_regex
void compile_regex(char **param_1) {
  char *__s;
  re_pattern_buffer *__buffer;
  size_t __length;
  char *pcVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined1 *puVar4;
  
  __buffer = (re_pattern_buffer *)(param_1 + 1);
  __s = *param_1;
  __buffer->buffer = (uchar *)0x0;
  param_1[2] = (char *)0x0;
  param_1[5] = (char *)(param_1 + 9);
  if (ignore_case == '\0') {
    puVar4 = (char *)0x0;
  }
  else {
    puVar4 = folded_chars;
  }
  param_1[6] = puVar4;
  __length = strlen(__s);
  pcVar1 = re_compile_pattern(__s,__length,__buffer);
  if (pcVar1 != (char *)0x0) {
    uVar2 = quote(__s);
    uVar3 = gettext("%s (for regexp %s)");
    error(1,0,uVar3,pcVar1,uVar2);
  }
  re_compile_fastmap(__buffer);
  return;
}

// Function: initialize_regex
void initialize_regex(void) {
  int iVar1;
  ushort **ppuVar2;
  int local_c;
  
  if (ignore_case != '\0') {
    for (local_c = 0; local_c < 0x100; local_c = local_c + 1) {
      iVar1 = toupper(local_c);
      folded_chars[local_c] = (char)iVar1;
    }
  }
  if (context_regex == (char *)0x0) {
    if ((gnu_extensions == '\0') || (input_reference == '\x01')) {
      context_regex = "\n";
    }
    else {
      context_regex = "[.?!][]\"\')}]*\\($\\|\t\\|  \\)[ \t\n]*";
    }
  }

// Function: swallow_file_in_memory
void swallow_file_in_memory(char *param_1,long *param_2) {
  char cVar1;
  bool bVar2;
  long lVar3;
  undefined8 uVar4;
  int *piVar5;
  long in_FS_OFFSET;
  long local_30;
  undefined *local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (((param_1 == (char *)0x0) || (*param_1 == '\0')) ||
     (cVar1 = streq(param_1,&DAT_001047ba), cVar1 != '\0')) {
    bVar2 = true;
  }
  else {
    bVar2 = false;
  }
  if (bVar2) {
    lVar3 = fread_file(_stdin,0,&local_30);
    *param_2 = lVar3;
  }
  else {
    lVar3 = read_file(param_1,0,&local_30);
    *param_2 = lVar3;
  }
  if (*param_2 == 0) {
    if (bVar2) {
      param_1 = "-";
    }
    uVar4 = quotearg_n_style_colon(0,3,param_1);
    piVar5 = __errno_location();
    local_28 = &DAT_001047bc;
    error(1,*piVar5,&DAT_001047bc,uVar4);
  }
  if (bVar2) {
    clearerr_unlocked(_stdin);
  }
  param_2[1] = *param_2 + local_30;
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: compare_words
int compare_words(long *param_1,long *param_2) {
  byte bVar1;
  byte bVar2;
  long lVar3;
  long local_38;
  long local_30;
  
  lVar3 = param_1[1];
  if (param_2[1] <= param_1[1]) {
    lVar3 = param_2[1];
  }
  if (ignore_case == '\0') {
    for (local_30 = 0; local_30 < lVar3; local_30 = local_30 + 1) {
      bVar1 = to_uchar((int)*(char *)(local_30 + *param_1));
      bVar2 = to_uchar((int)*(char *)(local_30 + *param_2));
      if ((uint)bVar1 - (uint)bVar2 != 0) {
        return (uint)bVar1 - (uint)bVar2;
      }
    }
  }
  else {
    for (local_38 = 0; local_38 < lVar3; local_38 = local_38 + 1) {
      bVar1 = to_uchar((int)*(char *)(local_38 + *param_1));
      bVar1 = folded_chars[(int)(uint)bVar1];
      bVar2 = to_uchar((int)*(char *)(local_38 + *param_2));
      if ((uint)bVar1 - (uint)(byte)folded_chars[(int)(uint)bVar2] != 0) {
        return (uint)bVar1 - (uint)(byte)folded_chars[(int)(uint)bVar2];
      }
    }
  }
  return (uint)(param_2[1] < param_1[1]) - (uint)(param_1[1] < param_2[1]);
}

// Function: compare_occurs
int compare_occurs(ulong *param_1,ulong *param_2) {
  int iVar1;
  
  iVar1 = compare_words(param_1,param_2);
  if (iVar1 == 0) {
    iVar1 = (uint)(*param_2 < *param_1) - (uint)(*param_1 < *param_2);
  }
  return iVar1;
}

// Function: search_table
undefined8 search_table(undefined8 param_1,long *param_2) {
  int iVar1;
  long lVar2;
  long local_20;
  long local_18;
  
  local_20 = 0;
  lVar2 = param_2[2];
  while( true ) {
    do {
      local_18 = lVar2;
      if (local_18 <= local_20) {
        return 0;
      }
      lVar2 = (ulong)((uint)local_20 & (uint)local_18 & 1) + (local_20 >> 1) + (local_18 >> 1);
      iVar1 = compare_words(param_1,lVar2 * 0x10 + *param_2);
    } while (iVar1 < 0);
    if (iVar1 < 1) break;
    local_20 = lVar2 + 1;
    lVar2 = local_18;
  }
  return 1;
}

// Function: sort_found_occurs
void sort_found_occurs(void) {
  if (number_of_occurs != 0) {
    qsort(occurs_table,number_of_occurs,0x30,compare_occurs);
  }
  return;
}

// Function: digest_break_file
void digest_break_file(undefined8 param_1) {
  byte bVar1;
  long in_FS_OFFSET;
  char *local_30;
  char *local_28;
  char *local_20;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  swallow_file_in_memory(param_1,&local_28);
  memset(word_fastmap,1,0x100);
  for (local_30 = local_28; local_30 < local_20; local_30 = local_30 + 1) {
    bVar1 = to_uchar((int)*local_30);
    word_fastmap[(int)(uint)bVar1] = 0;
  }
  if (gnu_extensions != '\x01') {
    word_fastmap[32] = 0;
    word_fastmap[9] = 0;
    word_fastmap[10] = 0;
  }
  free(local_28);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: digest_word_file
void digest_word_file(undefined8 param_1,void **param_2) {
  char *pcVar1;
  void *pvVar2;
  long in_FS_OFFSET;
  char *local_38;
  char *local_28;
  char *local_20;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  swallow_file_in_memory(param_1,&local_28);
  *param_2 = (void *)0x0;
  param_2[1] = (void *)0x0;
  param_2[2] = (void *)0x0;
  local_38 = local_28;
  while (pcVar1 = local_38, local_38 < local_20) {
    for (; (local_38 < local_20 && (*local_38 != '\n')); local_38 = local_38 + 1) {
    }
    if (pcVar1 < local_38) {
      if (param_2[2] == param_2[1]) {
        pvVar2 = (void *)xpalloc(*param_2,param_2 + 1,1,0xffffffffffffffff,0x10);
        *param_2 = pvVar2;
      }
      *(char **)((long)*param_2 + (long)param_2[2] * 0x10) = pcVar1;
      *(long *)((long)*param_2 + (long)param_2[2] * 0x10 + 8) = (long)local_38 - (long)pcVar1;
      param_2[2] = (void *)((long)param_2[2] + 1);
    }
    if (local_38 < local_20) {
      local_38 = local_38 + 1;
    }
  }
  qsort(*param_2,(size_t)param_2[2],0x10,compare_words);
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: find_occurs_in_text
void find_occurs_in_text(int param_1) {
  ushort *puVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  char **ppcVar5;
  ushort **ppuVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  char **ppcVar9;
  long in_FS_OFFSET;
  char *local_a0;
  char *local_98;
  char *local_90;
  char *local_88;
  long local_80;
  char *local_78;
  char *local_70;
  char *local_68;
  char *local_60;
  char *local_58;
  char *local_38;
  char *local_30;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  ppcVar5 = (char **)(text_buffers + (long)param_1 * 0x10);
  local_80 = 0;
  local_90 = *ppcVar5;
  local_88 = local_90;
  if (input_reference != '\0') {
    for (; local_88 < ppcVar5[1]; local_88 = local_88 + 1) {
      ppuVar6 = __ctype_b_loc();
      puVar1 = *ppuVar6;
      bVar2 = to_uchar((int)*local_88);
      if ((puVar1[bVar2] & 0x2000) != 0) break;
    }
    local_80 = (long)local_88 - (long)local_90;
    for (; local_88 < ppcVar5[1]; local_88 = local_88 + 1) {
      ppuVar6 = __ctype_b_loc();
      puVar1 = *ppuVar6;
      bVar2 = to_uchar((int)*local_88);
      if ((puVar1[bVar2] & 0x2000) == 0) break;
    }
  }
  local_a0 = *ppcVar5;
LAB_001016d0:
  if (ppcVar5[1] <= local_a0) {
    if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
      return;
    }
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  local_78 = local_a0;
  local_58 = ppcVar5[1];
  if (context_regex == 0) goto LAB_00101146;
  iVar4 = re_search((re_pattern_buffer *)&DAT_00103d28,local_a0,(int)ppcVar5[1] - (int)local_a0,0,
                    (int)ppcVar5[1] - (int)local_a0,(re_registers *)context_regs);
  if (iVar4 == 0) {
    uVar7 = quote(context_regex);
    uVar8 = gettext("error: regular expression has a match of length zero: %s");
    error(1,0,uVar8,uVar7);
  }
  else if (iVar4 < 1) {
    if (iVar4 == -2) {
      matcher_error();
      goto LAB_00101146;
    }
    if (iVar4 == -1) goto LAB_00101146;
  }
  local_58 = local_a0 + *(int *)context_regs._16_8_;
LAB_00101146:
  for (local_70 = local_58; local_a0 < local_70; local_70 = local_70 + -1) {
    ppuVar6 = __ctype_b_loc();
    puVar1 = *ppuVar6;
    bVar2 = to_uchar((int)local_70[-1]);
    if ((puVar1[bVar2] & 0x2000) == 0) break;
  }
LAB_00101196:
  do {
    if (word_regex == 0) {
      local_98 = local_a0;
      while ((local_98 < local_70 &&
             (bVar2 = to_uchar((int)*local_98), word_fastmap[(int)(uint)bVar2] == '\0'))) {
        local_98 = local_98 + 1;
      }
      if (local_98 == local_70) break;
      local_68 = local_98;
      while ((local_98 < local_70 &&
             (bVar2 = to_uchar((int)*local_98), word_fastmap[(int)(uint)bVar2] != '\0'))) {
        local_98 = local_98 + 1;
      }
      local_60 = local_98;
    }
    else {
      iVar4 = re_search((re_pattern_buffer *)&DAT_00103e88,local_a0,(int)local_70 - (int)local_a0,0,
                        (int)local_70 - (int)local_a0,(re_registers *)word_regs);
      if (iVar4 == -2) {
        matcher_error();
      }
      if (iVar4 == -1) break;
      local_68 = local_a0 + *(int *)word_regs._8_8_;
      local_60 = local_a0 + *(int *)word_regs._16_8_;
    }
    if (local_60 != local_68) {
      local_38 = local_68;
      local_30 = local_60 + -(long)local_68;
      local_a0 = local_68 + (long)local_30;
      if ((long)maximum_word_length < (long)local_30) {
        maximum_word_length = local_30;
      }
      if (input_reference != '\0') {
        while (local_88 < local_38) {
          if (*local_88 == '\n') {
            total_line_count = total_line_count + 1;
            local_90 = local_88 + 1;
            for (local_88 = local_90; local_88 < ppcVar5[1]; local_88 = local_88 + 1) {
              ppuVar6 = __ctype_b_loc();
              puVar1 = *ppuVar6;
              bVar2 = to_uchar((int)*local_88);
              if ((puVar1[bVar2] & 0x2000) != 0) break;
            }
            local_80 = (long)local_88 - (long)local_90;
          }
          else {
            local_88 = local_88 + 1;
          }
        }
        if (local_38 < local_88) goto LAB_00101196;
      }
      if (((ignore_file == 0) || (cVar3 = search_table(&local_38,ignore_table), cVar3 == '\0')) &&
         ((only_file == 0 || (cVar3 = search_table(&local_38,only_table), cVar3 == '\x01')))) {
        if (number_of_occurs == occurs_alloc) {
          occurs_table = xpalloc(occurs_table,&occurs_alloc,1,0xffffffffffffffff,0x30);
        }
        ppcVar9 = (char **)(number_of_occurs * 0x30 + occurs_table);
        if (auto_reference == '\0') {
          if ((input_reference != '\0') &&
             (ppcVar9[4] = local_90 + -(long)local_38, reference_max_width < local_80)) {
            reference_max_width = local_80;
          }
        }
        else {
LAB_00101556:
          if (local_88 < local_38) {
            if (*local_88 == '\n') {
              total_line_count = total_line_count + 1;
              local_90 = local_88 + 1;
              for (local_88 = local_90; local_88 < ppcVar5[1]; local_88 = local_88 + 1) {
                ppuVar6 = __ctype_b_loc();
                puVar1 = *ppuVar6;
                bVar2 = to_uchar((int)*local_88);
                if ((puVar1[bVar2] & 0x2000) != 0) break;
              }
            }
            else {
              local_88 = local_88 + 1;
            }
            goto LAB_00101556;
          }
          ppcVar9[4] = total_line_count;
        }
        if ((input_reference != '\0') && (local_90 == local_78)) {
          for (; local_78 < local_70; local_78 = local_78 + 1) {
            ppuVar6 = __ctype_b_loc();
            puVar1 = *ppuVar6;
            bVar2 = to_uchar((int)*local_78);
            if ((puVar1[bVar2] & 0x2000) != 0) break;
          }
          do {
            if (local_70 <= local_78) break;
            ppuVar6 = __ctype_b_loc();
            puVar1 = *ppuVar6;
            bVar2 = to_uchar((int)*local_78);
            if ((puVar1[bVar2] & 0x2000) == 0) break;
            local_78 = local_78 + 1;
          } while( true );
        }
        *ppcVar9 = local_38;
        ppcVar9[1] = local_30;
        ppcVar9[2] = local_78 + -(long)local_38;
        ppcVar9[3] = local_70 + -(long)local_38;
        *(int *)(ppcVar9 + 5) = param_1;
        number_of_occurs = number_of_occurs + 1;
      }
      goto LAB_00101196;
    }
    local_a0 = local_68 + 1;
  } while( true );
  local_a0 = local_58;
  goto LAB_001016d0;
}

// Function: print_spaces
void print_spaces(long param_1) {
  long local_10;
  
  for (local_10 = param_1; 0 < local_10; local_10 = local_10 + -1) {
    putchar_unlocked(0x20);
  }
  return;
}

// Function: print_field
void print_field(byte *param_1,byte *param_2) {
  byte bVar1;
  uint uVar2;
  byte *local_10;
  
  local_10 = param_1;
  do {
    if (param_2 <= local_10) {
      return;
    }
    bVar1 = *local_10;
    if (edited_flag[(int)(uint)bVar1] == '\0') {
      putchar_unlocked((int)(char)*local_10);
      goto LAB_0010185c;
    }
    if (bVar1 == 0x7d) {
LAB_001017fe:
      printf("$\\%c$",(ulong)bVar1);
    }
    else {
      if (bVar1 < 0x7e) {
        if (bVar1 == 0x7b) goto LAB_001017fe;
        if (bVar1 < 0x7c) {
          if (bVar1 == 0x5f) {
LAB_001017e7:
            putchar_unlocked(0x5c);
            putchar_unlocked((uint)bVar1);
            goto LAB_0010185c;
          }
          if (bVar1 < 0x60) {
            uVar2 = (uint)bVar1;
            if (uVar2 == 0x5c) {
              fwrite_unlocked("\\backslash{}",1,0xc,_stdout);
              goto LAB_0010185c;
            }
            if (uVar2 < 0x5d) {
              if (uVar2 == 0x22) {
                putchar_unlocked(0x22);
                putchar_unlocked(0x22);
                goto LAB_0010185c;
              }
              if ((0x21 < uVar2) && (uVar2 - 0x23 < 4)) goto LAB_001017e7;
            }
          }
        }
      }
      putchar_unlocked(0x20);
    }
LAB_0010185c:
    local_10 = local_10 + 1;
  } while( true );
}

// Function: fix_output_parameters
void fix_output_parameters(void) {
  byte bVar1;
  int iVar2;
  size_t sVar3;
  long lVar4;
  ushort **ppuVar5;
  long in_FS_OFFSET;
  int local_48;
  int local_44;
  long local_40;
  long local_38;
  char *local_30;
  char local_28 [24];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if (auto_reference != '\0') {
    reference_max_width = 0;
    for (local_48 = 0; local_48 < number_input_files; local_48 = local_48 + 1) {
      local_40 = *(long *)(file_line_count + (long)local_48 * 8) + 1;
      if (0 < local_48) {
        local_40 = local_40 - *(long *)(file_line_count + (long)local_48 * 8 + -8);
      }
      iVar2 = sprintf(local_28,"%jd",local_40);
      local_38 = (long)iVar2;
      if (*(long *)(input_file_name + (long)local_48 * 8) != 0) {
        sVar3 = strlen(*(char **)(input_file_name + (long)local_48 * 8));
        local_38 = sVar3 + local_38;
      }
      if (reference_max_width < local_38) {
        reference_max_width = local_38;
      }
    }
    lVar4 = reference_max_width + 2;
    reference_max_width = reference_max_width + 1;
    reference = xmalloc(lVar4);
  }
  if ((((auto_reference != '\0') || (input_reference != '\0')) && (right_reference != '\x01')) &&
     (line_width = line_width - (gap_size + reference_max_width), line_width < 0)) {
    line_width = 0;
  }
  half_line_width = line_width >> 1;
  before_max_width = half_line_width - gap_size;
  keyafter_max_width = half_line_width;
  if ((truncation_string == (undefined *)0x0) || (*truncation_string == '\0')) {
    truncation_string = (undefined *)0x0;
  }
  else {
    truncation_string_length = strlen(truncation_string);
  }
  if (gnu_extensions == '\0') {
    lVar4 = -(truncation_string_length * 2 + 1);
  }
  else {
    before_max_width = before_max_width + truncation_string_length * -2;
    if (before_max_width < 0) {
      before_max_width = 0;
    }
    lVar4 = truncation_string_length * -2;
  }
  keyafter_max_width = keyafter_max_width + lVar4;
  for (local_44 = 0; local_44 < 0x100; local_44 = local_44 + 1) {
    ppuVar5 = __ctype_b_loc();
    edited_flag[local_44] = ((*ppuVar5)[local_44] & 0x2000) != 0;
  }
  edited_flag[12] = 1;
  if (output_format == 3) {
    for (local_30 = "$%&#_{}\\"; *local_30 != '\0'; local_30 = local_30 + 1) {
      bVar1 = to_uchar((int)*local_30);
      edited_flag[(int)(uint)bVar1] = 1;
    }
  }
  else if (((output_format < 4) && (1 < output_format)) && (output_format == 2)) {
    edited_flag[34] = 1;
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: define_all_fields
void define_all_fields(char **param_1) {
  char *pcVar1;
  ushort *puVar2;
  byte bVar3;
  int iVar4;
  char *pcVar5;
  char *pcVar6;
  ushort **ppuVar7;
  long lVar8;
  long lVar9;
  char *pcVar10;
  char *local_70;
  char *local_68;
  char *local_60;
  char *local_58;
  
  keyafter = *param_1;
  DAT_00104428 = param_1[1] + (long)keyafter;
  pcVar5 = param_1[2] + (long)keyafter;
  pcVar6 = param_1[3] + (long)keyafter;
  pcVar10 = *(char **)((long)*(int *)(param_1 + 5) * 0x10 + text_buffers);
  pcVar1 = *(char **)((long)*(int *)(param_1 + 5) * 0x10 + text_buffers + 8);
  local_70 = DAT_00104428;
  while ((local_70 < pcVar6 && (local_70 <= keyafter + keyafter_max_width))) {
    DAT_00104428 = local_70;
    if (word_regex == 0) {
      bVar3 = to_uchar((int)*local_70);
      if (word_fastmap[(int)(uint)bVar3] == '\0') {
        local_70 = local_70 + 1;
      }
      else {
        while ((local_70 < pcVar6 &&
               (bVar3 = to_uchar((int)*local_70), word_fastmap[(int)(uint)bVar3] != '\0'))) {
          local_70 = local_70 + 1;
        }
      }
    }
    else {
      iVar4 = re_match((re_pattern_buffer *)&DAT_00103e88,local_70,(int)pcVar6 - (int)local_70,0,
                       (re_registers *)0x0);
      if (iVar4 == -2) {
        matcher_error();
      }
      if (iVar4 == -1) {
        lVar8 = 1;
      }
      else {
        lVar8 = (long)iVar4;
      }
      local_70 = local_70 + lVar8;
    }
  }
  if (local_70 <= keyafter + keyafter_max_width) {
    DAT_00104428 = local_70;
  }
  if ((truncation_string == (undefined *)0x0) || (pcVar6 <= DAT_00104428)) {
    keyafter_truncation = 0;
  }
  else {
    keyafter_truncation = 1;
  }
  for (; keyafter < DAT_00104428; DAT_00104428 = DAT_00104428 + -1) {
    ppuVar7 = __ctype_b_loc();
    puVar2 = *ppuVar7;
    bVar3 = to_uchar((int)DAT_00104428[-1]);
    if ((puVar2[bVar3] & 0x2000) == 0) break;
  }
  if ((char *)-(maximum_word_length + half_line_width) == param_1[2] ||
      -(long)param_1[2] < maximum_word_length + half_line_width) {
    local_68 = param_1[2] + (long)keyafter;
  }
  else {
    local_68 = keyafter + -(maximum_word_length + half_line_width);
    if (word_regex == 0) {
      bVar3 = to_uchar((int)*local_68);
      if (word_fastmap[(int)(uint)bVar3] == '\0') {
        local_68 = local_68 + 1;
      }
      else {
        while ((local_68 < keyafter &&
               (bVar3 = to_uchar((int)*local_68), word_fastmap[(int)(uint)bVar3] != '\0'))) {
          local_68 = local_68 + 1;
        }
      }
    }
    else {
      iVar4 = re_match((re_pattern_buffer *)&DAT_00103e88,local_68,(int)keyafter - (int)local_68,0,
                       (re_registers *)0x0);
      if (iVar4 == -2) {
        matcher_error();
      }
      if (iVar4 == -1) {
        lVar8 = 1;
      }
      else {
        lVar8 = (long)iVar4;
      }
      local_68 = local_68 + lVar8;
    }
  }
  before = local_68;
  for (DAT_00104408 = keyafter; before < DAT_00104408; DAT_00104408 = DAT_00104408 + -1) {
    ppuVar7 = __ctype_b_loc();
    puVar2 = *ppuVar7;
    bVar3 = to_uchar((int)DAT_00104408[-1]);
    if ((puVar2[bVar3] & 0x2000) == 0) break;
  }
  while (before + before_max_width < DAT_00104408) {
    if (word_regex == 0) {
      bVar3 = to_uchar((int)*before);
      if (word_fastmap[(int)(uint)bVar3] == '\0') {
        before = before + 1;
      }
      else {
        while ((before < DAT_00104408 &&
               (bVar3 = to_uchar((int)*before), word_fastmap[(int)(uint)bVar3] != '\0'))) {
          before = before + 1;
        }
      }
    }
    else {
      iVar4 = re_match((re_pattern_buffer *)&DAT_00103e88,before,(int)DAT_00104408 - (int)before,0,
                       (re_registers *)0x0);
      if (iVar4 == -2) {
        matcher_error();
      }
      if (iVar4 == -1) {
        lVar8 = 1;
      }
      else {
        lVar8 = (long)iVar4;
      }
      before = before + lVar8;
    }
  }
  if (truncation_string == (undefined *)0x0) {
    before_truncation = 0;
  }
  else {
    for (local_70 = before; pcVar10 < local_70; local_70 = local_70 + -1) {
      ppuVar7 = __ctype_b_loc();
      puVar2 = *ppuVar7;
      bVar3 = to_uchar((int)local_70[-1]);
      if ((puVar2[bVar3] & 0x2000) == 0) break;
    }
    before_truncation = pcVar5 < local_70;
  }
  for (; before < pcVar1; before = before + 1) {
    ppuVar7 = __ctype_b_loc();
    puVar2 = *ppuVar7;
    bVar3 = to_uchar((int)*before);
    if ((puVar2[bVar3] & 0x2000) == 0) break;
  }
  lVar8 = (before_max_width - ((long)DAT_00104408 - (long)before)) - gap_size;
  if (lVar8 < 1) {
    tail = (char *)0x0;
    DAT_001043e8 = (char *)0x0;
    tail_truncation = 0;
  }
  else {
    for (tail = DAT_00104428; tail < pcVar1; tail = tail + 1) {
      ppuVar7 = __ctype_b_loc();
      puVar2 = *ppuVar7;
      bVar3 = to_uchar((int)*tail);
      if ((puVar2[bVar3] & 0x2000) == 0) break;
    }
    DAT_001043e8 = tail;
    local_70 = tail;
    while ((local_70 < pcVar6 && (local_70 < tail + lVar8))) {
      DAT_001043e8 = local_70;
      if (word_regex == 0) {
        bVar3 = to_uchar((int)*local_70);
        if (word_fastmap[(int)(uint)bVar3] == '\0') {
          local_70 = local_70 + 1;
        }
        else {
          while ((local_70 < pcVar6 &&
                 (bVar3 = to_uchar((int)*local_70), word_fastmap[(int)(uint)bVar3] != '\0'))) {
            local_70 = local_70 + 1;
          }
        }
      }
      else {
        iVar4 = re_match((re_pattern_buffer *)&DAT_00103e88,local_70,(int)pcVar6 - (int)local_70,0,
                         (re_registers *)0x0);
        if (iVar4 == -2) {
          matcher_error();
        }
        if (iVar4 == -1) {
          lVar9 = 1;
        }
        else {
          lVar9 = (long)iVar4;
        }
        local_70 = local_70 + lVar9;
      }
    }
    if (local_70 < tail + lVar8) {
      DAT_001043e8 = local_70;
    }
    if (tail < DAT_001043e8) {
      keyafter_truncation = 0;
      if ((truncation_string == (undefined *)0x0) || (pcVar6 <= DAT_001043e8)) {
        tail_truncation = 0;
      }
      else {
        tail_truncation = 1;
      }
    }
    else {
      tail_truncation = 0;
    }
    for (; tail < DAT_001043e8; DAT_001043e8 = DAT_001043e8 + -1) {
      ppuVar7 = __ctype_b_loc();
      puVar2 = *ppuVar7;
      bVar3 = to_uchar((int)DAT_001043e8[-1]);
      if ((puVar2[bVar3] & 0x2000) == 0) break;
    }
  }
  lVar8 = (keyafter_max_width - ((long)DAT_00104428 - (long)keyafter)) - gap_size;
  if (lVar8 < 1) {
    head = (char *)0x0;
    DAT_00104448 = (char *)0x0;
    head_truncation = 0;
  }
  else {
    for (DAT_00104448 = before; pcVar10 < DAT_00104448; DAT_00104448 = DAT_00104448 + -1) {
      ppuVar7 = __ctype_b_loc();
      puVar2 = *ppuVar7;
      bVar3 = to_uchar((int)DAT_00104448[-1]);
      if ((puVar2[bVar3] & 0x2000) == 0) break;
    }
    head = local_68;
    while (head + lVar8 < DAT_00104448) {
      if (word_regex == 0) {
        bVar3 = to_uchar((int)*head);
        if (word_fastmap[(int)(uint)bVar3] == '\0') {
          head = head + 1;
        }
        else {
          while ((head < DAT_00104448 &&
                 (bVar3 = to_uchar((int)*head), word_fastmap[(int)(uint)bVar3] != '\0'))) {
            head = head + 1;
          }
        }
      }
      else {
        iVar4 = re_match((re_pattern_buffer *)&DAT_00103e88,head,(int)DAT_00104448 - (int)head,0,
                         (re_registers *)0x0);
        if (iVar4 == -2) {
          matcher_error();
        }
        if (iVar4 == -1) {
          lVar9 = 1;
        }
        else {
          lVar9 = (long)iVar4;
        }
        head = head + lVar9;
      }
    }
    if (head < DAT_00104448) {
      before_truncation = 0;
      if ((truncation_string == (undefined *)0x0) || (head <= pcVar5)) {
        head_truncation = 0;
      }
      else {
        head_truncation = 1;
      }
    }
    else {
      head_truncation = 0;
    }
    for (; head < DAT_00104448; head = head + 1) {
      ppuVar7 = __ctype_b_loc();
      puVar2 = *ppuVar7;
      bVar3 = to_uchar((int)*head);
      if ((puVar2[bVar3] & 0x2000) == 0) break;
    }
  }
  if (auto_reference == '\0') {
    if (input_reference != '\0') {
      reference = param_1[4] + (long)keyafter;
      for (DAT_00104468 = reference; DAT_00104468 < pcVar6; DAT_00104468 = DAT_00104468 + 1) {
        ppuVar7 = __ctype_b_loc();
        puVar2 = *ppuVar7;
        bVar3 = to_uchar((int)*DAT_00104468);
        if ((puVar2[bVar3] & 0x2000) != 0) {
          return;
        }
      }
    }
  }
  else {
    local_60 = *(char **)((long)*(int *)(param_1 + 5) * 8 + input_file_name);
    if (local_60 == (char *)0x0) {
      local_60 = "";
    }
    local_58 = param_1[4] + 1;
    if (0 < *(int *)(param_1 + 5)) {
      local_58 = local_58 + -*(long *)((long)*(int *)(param_1 + 5) * 8 + -8 + file_line_count);
    }
    pcVar10 = stpcpy(reference,local_60);
    iVar4 = sprintf(pcVar10,":%jd",local_58);
    DAT_00104468 = pcVar10 + iVar4;
  }
  return;
}

// Function: output_one_roff_line
void output_one_roff_line(void) {
  printf(".%s \"",macro_name);
  print_field(tail,DAT_001043e8);
  if (tail_truncation != '\0') {
    fputs_unlocked(truncation_string,_stdout);
  }
  putchar_unlocked(0x22);
  fwrite_unlocked(&DAT_00104824,1,2,_stdout);
  if (before_truncation != '\0') {
    fputs_unlocked(truncation_string,_stdout);
  }
  print_field(before,DAT_00104408);
  putchar_unlocked(0x22);
  fwrite_unlocked(&DAT_00104824,1,2,_stdout);
  print_field(keyafter,DAT_00104428);
  if (keyafter_truncation != '\0') {
    fputs_unlocked(truncation_string,_stdout);
  }
  putchar_unlocked(0x22);
  fwrite_unlocked(&DAT_00104824,1,2,_stdout);
  if (head_truncation != '\0') {
    fputs_unlocked(truncation_string,_stdout);
  }
  print_field(head,DAT_00104448);
  putchar_unlocked(0x22);
  if ((auto_reference != '\0') || (input_reference != '\0')) {
    fwrite_unlocked(&DAT_00104824,1,2,_stdout);
    print_field(reference,DAT_00104468);
    putchar_unlocked(0x22);
  }
  putchar_unlocked(10);
  return;
}

// Function: output_one_tex_line
void output_one_tex_line(void) {
  char *pcVar1;
  char *pcVar2;
  byte bVar3;
  int iVar4;
  long lVar5;
  char *local_30;
  
  printf("\\%s ",macro_name);
  putchar_unlocked(0x7b);
  print_field(tail,DAT_001043e8);
  fwrite_unlocked(&DAT_0010482c,1,2,_stdout);
  print_field(before,DAT_00104408);
  fwrite_unlocked(&DAT_0010482c,1,2,_stdout);
  pcVar2 = DAT_00104428;
  pcVar1 = keyafter;
  local_30 = keyafter;
  if (word_regex == 0) {
    bVar3 = to_uchar((int)*keyafter);
    if (word_fastmap[(int)(uint)bVar3] == '\0') {
      local_30 = pcVar1 + 1;
    }
    else {
      while ((local_30 < DAT_00104428 &&
             (bVar3 = to_uchar((int)*local_30), word_fastmap[(int)(uint)bVar3] != '\0'))) {
        local_30 = local_30 + 1;
      }
    }
  }
  else {
    iVar4 = re_match((re_pattern_buffer *)&DAT_00103e88,keyafter,(int)DAT_00104428 - (int)keyafter,0
                     ,(re_registers *)0x0);
    if (iVar4 == -2) {
      matcher_error();
    }
    if (iVar4 == -1) {
      lVar5 = 1;
    }
    else {
      lVar5 = (long)iVar4;
    }
    local_30 = pcVar1 + lVar5;
  }
  print_field(pcVar1,local_30);
  fwrite_unlocked(&DAT_0010482c,1,2,_stdout);
  print_field(local_30,pcVar2);
  fwrite_unlocked(&DAT_0010482c,1,2,_stdout);
  print_field(head,DAT_00104448);
  putchar_unlocked(0x7d);
  if ((auto_reference != '\0') || (input_reference != '\0')) {
    putchar_unlocked(0x7b);
    print_field(reference,DAT_00104468);
    putchar_unlocked(0x7d);
  }
  putchar_unlocked(10);
  return;
}

// Function: output_one_dumb_line
void output_one_dumb_line(void) {
  long lVar1;
  long lVar2;
  
  if (right_reference != '\x01') {
    if (auto_reference == '\0') {
      print_field(reference,DAT_00104468);
      print_spaces((reference_max_width + gap_size) - (DAT_00104468 - reference));
    }
    else {
      print_field(reference,DAT_00104468);
      putchar_unlocked(0x3a);
      print_spaces(((reference_max_width + gap_size) - (DAT_00104468 - reference)) + -1);
    }
  }
  if (tail < DAT_001043e8) {
    print_field(tail,DAT_001043e8);
    if (tail_truncation != '\0') {
      fputs_unlocked(truncation_string,_stdout);
    }
    lVar2 = truncation_string_length;
    if (before_truncation == '\0') {
      lVar2 = 0;
    }
    lVar1 = truncation_string_length;
    if (tail_truncation == '\0') {
      lVar1 = 0;
    }
    print_spaces(((((half_line_width - gap_size) - (DAT_00104408 - before)) - lVar2) -
                 (DAT_001043e8 - tail)) - lVar1);
  }
  else {
    lVar2 = truncation_string_length;
    if (before_truncation == '\0') {
      lVar2 = 0;
    }
    print_spaces(((half_line_width - gap_size) - (DAT_00104408 - before)) - lVar2);
  }
  if (before_truncation != '\0') {
    fputs_unlocked(truncation_string,_stdout);
  }
  print_field(before,DAT_00104408);
  print_spaces(gap_size);
  print_field(keyafter,DAT_00104428);
  if (keyafter_truncation != '\0') {
    fputs_unlocked(truncation_string,_stdout);
  }
  if (head < DAT_00104448) {
    lVar2 = truncation_string_length;
    if (keyafter_truncation == '\0') {
      lVar2 = 0;
    }
    lVar1 = truncation_string_length;
    if (head_truncation == '\0') {
      lVar1 = 0;
    }
    print_spaces((((half_line_width - (DAT_00104428 - keyafter)) - lVar2) - (DAT_00104448 - head)) -
                 lVar1);
    if (head_truncation != '\0') {
      fputs_unlocked(truncation_string,_stdout);
    }
    print_field(head,DAT_00104448);
  }
  else if (((auto_reference != '\0') || (input_reference != '\0')) && (right_reference != '\0')) {
    lVar2 = truncation_string_length;
    if (keyafter_truncation == '\0') {
      lVar2 = 0;
    }
    print_spaces((half_line_width - (DAT_00104428 - keyafter)) - lVar2);
  }
  if (((auto_reference != '\0') || (input_reference != '\0')) && (right_reference != '\0')) {
    print_spaces(gap_size);
    print_field(reference,DAT_00104468);
  }
  putchar_unlocked(10);
  return;
}

// Function: generate_all_output
void generate_all_output(void) {
  long local_18;
  long local_10;
  
  tail = 0;
  DAT_001043e8 = 0;
  tail_truncation = 0;
  head = 0;
  DAT_00104448 = 0;
  head_truncation = 0;
  local_18 = occurs_table;
  for (local_10 = 0; local_10 < number_of_occurs; local_10 = local_10 + 1) {
    define_all_fields(local_18);
    if (output_format == 3) {
      output_one_tex_line();
    }
    else if (output_format < 4) {
      if (output_format < 2) {
        output_one_dumb_line();
      }
      else if (output_format == 2) {
        output_one_roff_line();
      }
    }
    local_18 = local_18 + 0x30;
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
                            "Usage: %s [OPTION]... [INPUT]...   (without -G)\n  or:  %s -G [OPTION]... [INPUT [OUTPUT]]\n"
                            );
    printf(pcVar3,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Output a permuted index, including context, of the words in the input files.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_stdin_note();
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -A, --auto-reference           output automatically generated references\n  -G, --traditional              behave more like System V \'ptx\'\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -F, --flag-truncation=STRING   use STRING for flagging line truncations.\n                                 The default is \'/\'\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -M, --macro-name=STRING        macro name to use instead of \'xx\'\n  -O, --format=roff              generate output as roff directives\n  -R, --right-side-refs          put references at right, not counted in -w\n  -S, --sentence-regexp=REGEXP   for end of lines or end of sentences\n  -T, --format=tex               generate output as TeX directives\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -W, --word-regexp=REGEXP       use REGEXP to match each keyword\n  -b, --break-file=FILE          word break characters in this FILE\n  -f, --ignore-case              fold lower case to upper case for sorting\n  -g, --gap-size=NUMBER          gap size in columns between output fields\n  -i, --ignore-file=FILE         read ignore word list from FILE\n  -o, --only-file=FILE           read only word list from this FILE\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -r, --references               first field of each line is a reference\n  -t, --typeset-mode               - not implemented -\n  -w, --width=NUMBER             output width in columns, reference excluded\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_00104e59);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: main
undefined8 main(int param_1,undefined8 *param_2) {
  char cVar1;
  int iVar2;
  long lVar3;
  undefined8 uVar4;
  int *piVar5;
  long in_FS_OFFSET;
  int local_78;
  long local_50;
  long local_48;
  undefined8 local_40;
  undefined8 local_38;
  undefined *local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  do {
    while( true ) {
      iVar2 = getopt_long(param_1,param_2,"AF:GM:ORS:TW:b:i:fg:o:trw:",long_options,0);
      if (iVar2 == -1) {
        if (param_1 == _optind) {
          input_file_name = (undefined8 *)xmalloc(8);
          file_line_count = xmalloc(8);
          text_buffers = xmalloc(0x10);
          number_input_files = 1;
          *input_file_name = 0;
        }
        else if (gnu_extensions == '\0') {
          number_input_files = 1;
          input_file_name = (undefined8 *)xmalloc(8);
          file_line_count = xmalloc(8);
          text_buffers = xmalloc(0x10);
          if ((*(char *)param_2[_optind] == '\0') ||
             (cVar1 = streq(param_2[_optind],&DAT_001047ba), cVar1 != '\0')) {
            *input_file_name = 0;
          }
          else {
            *input_file_name = param_2[_optind];
          }
          _optind = _optind + 1;
          if (_optind < param_1) {
            lVar3 = freopen_safer(param_2[_optind],&DAT_00104fca,_stdout);
            if (lVar3 == 0) {
              uVar4 = quotearg_n_style_colon(0,3,param_2[_optind]);
              piVar5 = __errno_location();
              local_30 = &DAT_001047bc;
              error(1,*piVar5,&DAT_001047bc,uVar4);
            }
            _optind = _optind + 1;
          }
          if (_optind < param_1) {
            uVar4 = quote(param_2[_optind]);
            local_28 = gettext("extra operand %s");
            error(0,0,local_28,uVar4);
            usage(1);
          }
        }
        else {
          number_input_files = param_1 - _optind;
          input_file_name = (undefined8 *)xnmalloc((long)number_input_files,8);
          file_line_count = xnmalloc((long)number_input_files,8);
          text_buffers = xnmalloc((long)number_input_files,0x10);
          for (local_78 = 0; local_78 < number_input_files; local_78 = local_78 + 1) {
            if ((*(char *)param_2[_optind] == '\0') ||
               (cVar1 = streq(param_2[_optind],&DAT_001047ba), cVar1 != '\0')) {
              input_file_name[local_78] = 0;
            }
            else {
              input_file_name[local_78] = param_2[_optind];
            }
            _optind = _optind + 1;
          }
        }
        if (output_format == 0) {
          if (gnu_extensions == '\0') {
            output_format = 2;
          }
          else {
            output_format = 1;
          }
        }
        initialize_regex();
        if (break_file != (char *)0x0) {
          digest_break_file(break_file);
        }
        if ((ignore_file != (char *)0x0) &&
           (digest_word_file(ignore_file,ignore_table), ignore_table._16_8_ == 0)) {
          ignore_file = (char *)0x0;
        }
        if ((only_file != (char *)0x0) &&
           (digest_word_file(only_file,only_table), only_table._16_8_ == 0)) {
          only_file = (char *)0x0;
        }
        number_of_occurs = 0;
        total_line_count = 0;
        maximum_word_length = 0;
        reference_max_width = 0;
        for (local_78 = 0; local_78 < number_input_files; local_78 = local_78 + 1) {
          local_48 = text_buffers + (long)local_78 * 0x10;
          swallow_file_in_memory(input_file_name[local_78],local_48);
          find_occurs_in_text(local_78);
          total_line_count = total_line_count + 1;
          *(long *)((long)local_78 * 8 + file_line_count) = total_line_count;
        }
        sort_found_occurs();
        fix_output_parameters();
        generate_all_output();
        if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        return 0;
      }
      if (iVar2 < 0x78) break;
switchD_00103420_caseD_b:
      usage(1);
switchD_00103420_caseD_47:
      gnu_extensions = '\0';
    }
    if (iVar2 < 10) {
      if (iVar2 == -0x83) {
LAB_001036e0:
        uVar4 = proper_name_lite("F. Pinard",&DAT_00104f86);
        version_etc(_stdout,&DAT_00104e59,"GNU coreutils",_Version,uVar4,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar2 == -0x82) {
        usage(0);
        goto LAB_001036e0;
      }
      goto switchD_00103420_caseD_b;
    }
    switch(iVar2) {
    case 10:
      lVar3 = __xargmatch_internal("--format",_optarg,format_args,&format_vals,4,_argmatch_die,1);
      output_format = *(int *)((long)&format_vals + lVar3 * 4);
      break;
    default:
      goto switchD_00103420_caseD_b;
    case 0x41:
      auto_reference = 1;
      break;
    case 0x46:
      truncation_string = _optarg;
      unescape_string(_optarg);
      break;
    case 0x47:
      goto switchD_00103420_caseD_47;
    case 0x4d:
      macro_name = _optarg;
      break;
    case 0x4f:
      output_format = 2;
      break;
    case 0x52:
      right_reference = 1;
      break;
    case 0x53:
      context_regex = _optarg;
      unescape_string(_optarg);
      break;
    case 0x54:
      output_format = 3;
      break;
    case 0x57:
      word_regex = _optarg;
      unescape_string(_optarg);
      if (*word_regex == '\0') {
        word_regex = (char *)0x0;
      }
      break;
    case 0x62:
      break_file = _optarg;
      break;
    case 0x66:
      ignore_case = 1;
      break;
    case 0x67:
      iVar2 = xstrtoimax(_optarg,0,0,&local_50,&DAT_001045fc);
      if ((iVar2 != 0) || (local_50 < 1)) {
        uVar4 = quote(_optarg);
        local_40 = gettext("invalid gap width: %s");
        error(1,0,local_40,uVar4);
      }
      gap_size = local_50;
      break;
    case 0x69:
      ignore_file = _optarg;
      break;
    case 0x6f:
      only_file = _optarg;
      break;
    case 0x72:
      input_reference = 1;
      break;
    case 0x74:
      break;
    case 0x77:
      iVar2 = xstrtoimax(_optarg,0,0,&local_50,&DAT_001045fc);
      if ((iVar2 != 0) || (local_50 < 1)) {
        uVar4 = quote(_optarg);
        local_38 = gettext("invalid line width: %s");
        error(1,0,local_38,uVar4);
      }
      line_width = local_50;
    }
  } while( true );
}

