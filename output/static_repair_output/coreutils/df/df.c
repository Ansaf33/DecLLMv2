// Function: automount_stat_err
int automount_stat_err(char *param_1,stat *param_2) {
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar1 = open(param_1,0x900);
  if (iVar1 < 0) {
    piVar3 = __errno_location();
    if ((*piVar3 != 2) && (piVar3 = __errno_location(), *piVar3 != 0x14)) {
      iVar1 = stat(param_1,param_2);
      if (iVar1 != 0) {
        piVar3 = __errno_location();
        return *piVar3;
      }
      return 0;
    }
    piVar3 = __errno_location();
    iVar2 = *piVar3;
  }
  else {
    iVar2 = fstat(iVar1,param_2);
    if (iVar2 == 0) {
      iVar2 = 0;
    }
    else {
      piVar3 = __errno_location();
      iVar2 = *piVar3;
    }
    close(iVar1);
  }
  return iVar2;
}

// Function: replace_control_chars
void replace_control_chars(char *param_1) {
  char cVar1;
  char *local_10;
  
  for (local_10 = param_1; *local_10 != '\0'; local_10 = local_10 + 1) {
    cVar1 = c_iscntrl((int)*local_10);
    if (cVar1 != '\0') {
      *local_10 = '?';
    }
  }
  return;
}

// Function: replace_invalid_chars
void replace_invalid_chars(char *param_1) {
  int iVar1;
  size_t sVar2;
  long in_FS_OFFSET;
  bool bVar3;
  undefined4 local_44;
  char *local_40;
  ulong local_38;
  char *local_30;
  char *local_28;
  ulong local_20;
  undefined local_18 [8];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  sVar2 = strlen(param_1);
  local_28 = param_1 + sVar2;
  local_40 = param_1;
  mbszero(local_18);
  for (local_30 = param_1; local_30 != local_28; local_30 = local_30 + local_38) {
    local_20 = (long)local_28 - (long)local_30;
    local_38 = rpl_mbrtoc32(&local_44,local_30,local_20,local_18);
    bVar3 = local_38 <= local_20;
    if (bVar3) {
      iVar1 = c32iscntrl(local_44);
      bVar3 = iVar1 == 0;
    }
    else {
      local_38 = 1;
    }
    if (bVar3) {
      memmove(local_40,local_30,local_38);
      local_40 = local_40 + local_38;
    }
    else {
      *local_40 = '?';
      local_40 = local_40 + 1;
      mbszero(local_18);
    }
  }
  *local_40 = '\0';
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: replace_problematic_chars
void replace_problematic_chars(undefined8 param_1) {
  code *pcVar1;
  
  if (tty_out_3 < 0) {
    tty_out_3 = isatty(1);
  }
  if (tty_out_3 == 0) {
    pcVar1 = replace_control_chars;
  }
  else {
    pcVar1 = replace_invalid_chars;
  }
  (*pcVar1)(param_1);
  return;
}

// Function: alloc_table_row
void alloc_table_row(void) {
  undefined8 *puVar1;
  undefined8 uVar2;
  
  if (nrows == nrows_alloc) {
    table = xpalloc(table,&nrows_alloc,1,0xffffffffffffffff,8);
  }
  puVar1 = (undefined8 *)(table + nrows * 8);
  nrows = nrows + 1;
  uVar2 = xinmalloc(ncolumns,8);
  *puVar1 = uVar2;
  return;
}

// Function: print_table
void print_table(void) {
  char *__s;
  int local_28;
  long local_20;
  long local_18;
  
  for (local_20 = 0; local_20 < nrows; local_20 = local_20 + 1) {
    for (local_18 = 0; local_18 < ncolumns; local_18 = local_18 + 1) {
      __s = *(char **)(*(long *)(table + local_20 * 8) + local_18 * 8);
      if (local_18 != 0) {
        putchar_unlocked(0x20);
      }
      local_28 = gnu_mbswidth(__s,3);
      if (local_28 < 0) {
        local_28 = 0;
      }
      else {
        local_28 = *(int *)(*(long *)(columns + local_18 * 8) + 0x20) - local_28;
      }
      if (*(char *)(*(long *)(columns + local_18 * 8) + 0x24) != '\0') {
        for (; 0 < local_28; local_28 = local_28 + -1) {
          putchar_unlocked(0x20);
        }
      }
      fputs_unlocked(__s,_stdout);
      if (local_18 + 1 < ncolumns) {
        for (; 0 < local_28; local_28 = local_28 + -1) {
          putchar_unlocked(0x20);
        }
      }
    }
    putchar_unlocked(10);
  }
  return;
}

// Function: alloc_field
void alloc_field(int param_1,long param_2) {
  long lVar1;
  
  if (ncolumns == ncolumns_alloc) {
    columns = xpalloc(columns,&ncolumns_alloc,1,0xffffffffffffffff,8);
  }
  lVar1 = ncolumns * 8;
  ncolumns = ncolumns + 1;
  *(undefined1 **)(columns + lVar1) = field_data + (long)param_1 * 0x28;
  if (param_2 != 0) {
    *(long *)(field_data + (long)param_1 * 0x28 + 0x18) = param_2;
  }
  if (field_data[(long)param_1 * 0x28 + 0x25] == '\x01') {
                    /* WARNING: Subroutine does not return */
    __assert_fail("!field_data[f].used",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/df.c",
                  0x1a3,"alloc_field");
  }
  field_data[(long)param_1 * 0x28 + 0x25] = 1;
  return;
}

// Function: decode_output_arg
void decode_output_arg(undefined8 param_1) {
  char cVar1;
  char *__ptr;
  char *pcVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  uint local_5c;
  char *local_48;
  char *local_40;
  ulong local_38;
  
  __ptr = (char *)xstrdup(param_1);
  local_48 = __ptr;
  do {
    pcVar2 = strchr(local_48,0x2c);
    local_40 = pcVar2;
    if (pcVar2 != (char *)0x0) {
      local_40 = pcVar2 + 1;
      *pcVar2 = '\0';
    }
    local_5c = 0xc;
    for (local_38 = 0; local_38 < 0xc; local_38 = local_38 + 1) {
      cVar1 = streq(*(undefined8 *)(field_data + local_38 * 0x28 + 8),local_48);
      if (cVar1 != '\0') {
        local_5c = (uint)local_38;
        break;
      }
    }
    if (local_5c == 0xc) {
      uVar3 = quote(local_48);
      uVar4 = gettext("option --output: field %s unknown");
      error(0,0,uVar4,uVar3);
      usage(1);
    }
    if (field_data[(ulong)local_5c * 0x28 + 0x25] != '\0') {
      uVar3 = quote(*(undefined8 *)(field_data + (ulong)local_5c * 0x28 + 8));
      uVar4 = gettext("option --output: field %s used more than once");
      error(0,0,uVar4,uVar3);
      usage(1);
    }
    switch(local_5c) {
    case 0:
    case 1:
    case 3:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 0xb:
      alloc_field(local_5c,0);
      break;
    case 2:
      alloc_field(local_5c,&DAT_00104946);
      break;
    case 4:
      alloc_field(local_5c,"Avail");
      break;
    default:
                    /* WARNING: Subroutine does not return */
      __assert_fail("!\"invalid field\"",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/df.c",
                    0x1eb,"decode_output_arg");
    }
    local_48 = local_40;
    if (local_40 == (char *)0x0) {
      free(__ptr);
      return;
    }
  } while( true );
}

// Function: get_field_list
void get_field_list(void) {
                    /* WARNING: Could not recover jumptable at 0x00100b02. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)(&DAT_001049a0 + *(int *)(&DAT_001049a0 + (ulong)header_mode * 4)))();
  return;
}

// Function: get_header
void get_header(void) {
  int iVar1;
  ulong uVar2;
  int iVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  long in_FS_OFFSET;
  bool bVar7;
  bool bVar8;
  uint local_2f0;
  long local_2e8;
  undefined8 local_2e0;
  ulong local_2d8;
  ulong local_2d0;
  undefined local_2b8 [664];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  alloc_table_row();
  for (local_2e8 = 0; local_2e8 < ncolumns; local_2e8 = local_2e8 + 1) {
    uVar4 = gettext(*(undefined8 *)(*(long *)(columns + local_2e8 * 8) + 0x18));
    uVar2 = output_block_size;
    if ((**(int **)(columns + local_2e8 * 8) == 2) &&
       ((header_mode == 0 || ((header_mode == 4 && ((human_output_opts & 0x10) == 0)))))) {
      local_2f0 = human_output_opts & 0x124 | 0x98;
      local_2d8 = output_block_size;
      local_2d0 = output_block_size;
      do {
        bVar7 = local_2d8 % 1000 == 0;
        local_2d8 = local_2d8 / 1000;
        bVar8 = (local_2d0 & 0x3ff) == 0;
        local_2d0 = local_2d0 >> 10;
      } while (bVar8 && bVar7);
      if (bVar7 < bVar8) {
        local_2f0 = human_output_opts & 0x124 | 0xb8;
      }
      if (bVar8 < bVar7) {
        local_2f0 = local_2f0 & 0xffffffdf;
      }
      if ((local_2f0 & 0x20) == 0) {
        local_2f0 = local_2f0 | 0x100;
      }
      uVar4 = human_readable(output_block_size,local_2b8,local_2f0,1,1);
      uVar5 = gettext("blocks");
      uVar6 = gettext("%s-%s");
      local_2e0 = xasprintf(uVar6,uVar4,uVar5);
    }
    else if ((header_mode == 3) && (**(int **)(columns + local_2e8 * 8) == 2)) {
      uVar5 = gettext("%ju-%s");
      local_2e0 = xasprintf(uVar5,uVar2,uVar4);
    }
    else {
      local_2e0 = xstrdup(uVar4);
    }
    replace_problematic_chars(local_2e0);
    *(undefined8 *)(local_2e8 * 8 + *(long *)(table + nrows * 8 + -8)) = local_2e0;
    iVar3 = gnu_mbswidth(local_2e0,3);
    iVar1 = *(int *)(*(long *)(columns + local_2e8 * 8) + 0x20);
    if (iVar3 <= iVar1) {
      iVar3 = iVar1;
    }
    *(int *)(*(long *)(columns + local_2e8 * 8) + 0x20) = iVar3;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: selected_fstype
undefined8 selected_fstype(long param_1) {
  char cVar1;
  undefined8 uVar2;
  undefined8 *local_10;
  
  if ((fs_select_list == (undefined8 *)0x0) || (param_1 == 0)) {
    uVar2 = 1;
  }
  else {
    for (local_10 = fs_select_list; local_10 != (undefined8 *)0x0;
        local_10 = (undefined8 *)local_10[1]) {
      cVar1 = streq(param_1,*local_10);
      if (cVar1 != '\0') {
        return 1;
      }
    }
    uVar2 = 0;
  }
  return uVar2;
}

// Function: excluded_fstype
undefined8 excluded_fstype(long param_1) {
  char cVar1;
  undefined8 *local_10;
  
  if ((fs_exclude_list != (undefined8 *)0x0) && (param_1 != 0)) {
    for (local_10 = fs_exclude_list; local_10 != (undefined8 *)0x0;
        local_10 = (undefined8 *)local_10[1]) {
      cVar1 = streq(param_1,*local_10);
      if (cVar1 != '\0') {
        return 1;
      }
    }
  }
  return 0;
}

// Function: devlist_hash
ulong devlist_hash(ulong *param_1,ulong param_2) {
  return *param_1 % param_2;
}

// Function: devlist_compare
undefined8 devlist_compare(long *param_1,long *param_2) {
  return CONCAT71((int7)((ulong)*param_2 >> 8),*param_1 == *param_2);
}

// Function: devlist_for_dev
undefined8 devlist_for_dev(undefined8 param_1) {
  undefined8 uVar1;
  long lVar2;
  long in_FS_OFFSET;
  undefined8 local_38 [5];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if (devlist_table == 0) {
    uVar1 = 0;
  }
  else {
    local_38[0] = param_1;
    lVar2 = hash_lookup(devlist_table,local_38);
    if (lVar2 == 0) {
      uVar1 = 0;
    }
    else {
      uVar1 = *(undefined8 *)(lVar2 + 0x18);
    }
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar1;
}

// Function: filter_mount_list
void filter_mount_list(char param_1) {
  char **ppcVar1;
  char cVar2;
  bool bVar3;
  int iVar4;
  size_t sVar5;
  size_t sVar6;
  size_t sVar7;
  size_t sVar8;
  char *pcVar9;
  char **ppcVar10;
  long lVar11;
  long in_FS_OFFSET;
  int local_f4;
  char **local_f0;
  char **local_e8;
  char **local_e0;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_e8 = (char **)0x0;
  local_f4 = 0;
  for (local_f0 = mount_list; local_f0 != (char **)0x0; local_f0 = (char **)local_f0[6]) {
    local_f4 = local_f4 + 1;
  }
  devlist_table = hash_initialize((long)local_f4,0,devlist_hash,devlist_compare,0);
  if (devlist_table == 0) {
    xalloc_die();
  }
  local_f0 = mount_list;
  do {
    if (local_f0 == (char **)0x0) {
      if (param_1 != '\x01') {
        mount_list = (char **)0x0;
        while (local_e8 != (char **)0x0) {
          ppcVar10 = (char **)local_e8[1];
          ppcVar10[6] = (char *)mount_list;
          ppcVar1 = (char **)local_e8[2];
          mount_list = ppcVar10;
          free(local_e8);
          local_e8 = ppcVar1;
        }
        hash_free(devlist_table);
        devlist_table = 0;
      }
      if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return;
    }
    local_e0 = (char **)0x0;
    if ((((*(byte *)(local_f0 + 5) & 2) == 0) || (show_local_fs == '\0')) &&
       (((*(byte *)(local_f0 + 5) & 1) == 0 ||
        ((show_all_fs == '\x01' || (show_listed_fs == '\x01')))))) {
      cVar2 = selected_fstype(local_f0[3]);
      if (cVar2 != '\x01') goto LAB_001013dd;
      cVar2 = excluded_fstype(local_f0[3]);
      if (cVar2 != '\0') goto LAB_001013dd;
      iVar4 = stat(local_f0[1],&local_b8);
      if (iVar4 == -1) goto LAB_001013dd;
      lVar11 = devlist_for_dev(local_b8.st_dev);
      if (lVar11 != 0) {
        sVar5 = strlen(*(char **)(*(long *)(lVar11 + 8) + 8));
        sVar6 = strlen(local_f0[1]);
        if ((*(long *)(*(long *)(lVar11 + 8) + 0x10) == 0) || (local_f0[2] == (char *)0x0)) {
LAB_001014ae:
          bVar3 = false;
        }
        else {
          sVar7 = strlen(*(char **)(*(long *)(lVar11 + 8) + 0x10));
          sVar8 = strlen(local_f0[2]);
          if (sVar8 <= sVar7) goto LAB_001014ae;
          bVar3 = true;
        }
        if (((print_grand_total != '\x01') && ((*(byte *)(local_f0 + 5) & 2) != 0)) &&
           ((*(byte *)(*(long *)(lVar11 + 8) + 0x28) & 2) != 0)) {
          cVar2 = streq(**(undefined8 **)(lVar11 + 8),*local_f0);
          if (cVar2 != '\x01') goto LAB_001015fe;
        }
        pcVar9 = strchr(*local_f0,0x2f);
        if (pcVar9 == (char *)0x0) {
LAB_00101560:
          if ((sVar5 <= sVar6) || (bVar3)) {
            cVar2 = streq(**(undefined8 **)(lVar11 + 8),*local_f0);
            if (cVar2 != '\x01') {
              cVar2 = streq(local_f0[1],*(undefined8 *)(*(long *)(lVar11 + 8) + 8));
              if (cVar2 != '\0') goto LAB_001015ca;
            }
            local_e0 = local_f0;
            goto LAB_001015fe;
          }
        }
        else {
          pcVar9 = strchr(**(char ***)(lVar11 + 8),0x2f);
          if (pcVar9 != (char *)0x0) goto LAB_00101560;
        }
LAB_001015ca:
        local_e0 = *(char ***)(lVar11 + 8);
        *(char ***)(lVar11 + 8) = local_f0;
      }
    }
    else {
LAB_001013dd:
      local_b8.st_dev = (__dev_t)local_f0[4];
    }
LAB_001015fe:
    if (local_e0 == (char **)0x0) {
      ppcVar10 = (char **)xmalloc(0x20);
      ppcVar10[1] = (char *)local_f0;
      *ppcVar10 = (char *)local_b8.st_dev;
      ppcVar10[2] = (char *)local_e8;
      lVar11 = hash_insert(devlist_table,ppcVar10);
      if (lVar11 == 0) {
        xalloc_die();
      }
      *(char ***)(lVar11 + 0x18) = ppcVar10;
      local_f0 = (char **)local_f0[6];
      local_e8 = ppcVar10;
    }
    else {
      local_f0 = (char **)local_f0[6];
      if (param_1 != '\x01') {
        free_mount_entry(local_e0);
      }
    }
  } while( true );
}

// Function: me_for_dev
undefined8 me_for_dev(undefined8 param_1) {
  long lVar1;
  undefined8 uVar2;
  
  lVar1 = devlist_for_dev(param_1);
  if (lVar1 == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = *(undefined8 *)(lVar1 + 8);
  }
  return uVar2;
}

// Function: known_value
bool known_value(ulong param_1) {
  return param_1 < 0xfffffffffffffffe;
}

// Function: df_readable
undefined *
df_readable(byte param_1,long param_2,long param_3,undefined8 param_4,undefined8 param_5) {
  char cVar1;
  undefined *puVar2;
  
  cVar1 = known_value(param_2);
  if ((cVar1 == '\x01') || (param_1 == 1)) {
    if (param_1 != 0) {
      param_2 = -param_2;
    }
    puVar2 = (undefined *)
             human_readable(param_2,(ulong)param_1 + param_3,human_output_opts,param_4,param_5);
    if (param_1 != 0) {
      puVar2 = puVar2 + -1;
      *puVar2 = 0x2d;
    }
  }
  else {
    puVar2 = &DAT_001049c1;
  }
  return puVar2;
}

// Function: add_uint_with_neg_flag
void add_uint_with_neg_flag(ulong *param_1,char *param_2,ulong param_3,char param_4) {
  ulong local_20;
  
  if (param_4 == *param_2) {
    *param_1 = *param_1 + param_3;
  }
  else {
    if (*param_2 != '\0') {
      *param_1 = -*param_1;
    }
    local_20 = param_3;
    if (param_4 != '\0') {
      local_20 = -param_3;
    }
    if (local_20 < *param_1) {
      *param_1 = *param_1 - local_20;
    }
    else {
      *param_1 = local_20 - *param_1;
      *param_2 = param_4;
    }
    if (*param_2 != '\0') {
      *param_1 = -*param_1;
    }
  }
  return;
}

// Function: has_uuid_suffix
undefined4 has_uuid_suffix(char *param_1) {
  size_t sVar1;
  
  sVar1 = strlen(param_1);
  if ((0x24 < sVar1) &&
     (sVar1 = strspn(param_1 + (sVar1 - 0x24),"-0123456789abcdefABCDEF"), sVar1 == 0x24)) {
    return 1;
  }
  return 0;
}

// Function: get_field_values
void get_field_values(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3) {
  char cVar1;
  undefined uVar2;
  
  param_2[1] = 1;
  *param_2 = param_2[1];
  param_2[2] = param_3[5];
  param_2[5] = param_3[6];
  param_2[3] = param_2[5];
  *(undefined *)(param_2 + 4) = 0;
  param_2[6] = 0xffffffffffffffff;
  *(undefined *)(param_2 + 7) = 0;
  cVar1 = known_value(param_2[2]);
  if (cVar1 != '\0') {
    cVar1 = known_value(param_2[5]);
    if (cVar1 != '\0') {
      param_2[6] = param_2[2] - param_2[5];
      *(bool *)(param_2 + 7) = (ulong)param_2[2] < (ulong)param_2[5];
    }
  }
  *param_1 = *param_3;
  param_1[1] = output_block_size;
  param_1[2] = param_3[1];
  param_1[3] = param_3[3];
  param_1[5] = param_3[2];
  if (*(char *)(param_3 + 4) != '\0') {
    cVar1 = known_value(param_3[3]);
    if (cVar1 != '\0') {
      uVar2 = 1;
      goto LAB_00101b21;
    }
  }
  uVar2 = 0;
LAB_00101b21:
  *(undefined *)(param_1 + 4) = uVar2;
  param_1[6] = 0xffffffffffffffff;
  *(undefined *)(param_1 + 7) = 0;
  cVar1 = known_value(param_1[2]);
  if (cVar1 != '\0') {
    cVar1 = known_value(param_1[5]);
    if (cVar1 != '\0') {
      param_1[6] = param_1[2] - param_1[5];
      *(bool *)(param_1 + 7) = (ulong)param_1[2] < (ulong)param_1[5];
    }
  }
  return;
}

// Function: add_to_grand_total
void add_to_grand_total(long *param_1,long param_2) {
  char cVar1;
  
  cVar1 = known_value(*(undefined8 *)(param_2 + 0x10));
  if (cVar1 != '\0') {
    grand_fsu._40_8_ = *(long *)(param_2 + 0x10) + grand_fsu._40_8_;
  }
  cVar1 = known_value(*(undefined8 *)(param_2 + 0x18));
  if (cVar1 != '\0') {
    grand_fsu._48_8_ = *(long *)(param_2 + 0x18) + grand_fsu._48_8_;
  }
  cVar1 = known_value(param_1[2]);
  if (cVar1 != '\0') {
    grand_fsu._8_8_ = param_1[2] * *param_1 + grand_fsu._8_8_;
  }
  cVar1 = known_value(param_1[5]);
  if (cVar1 != '\0') {
    grand_fsu._16_8_ = param_1[5] * *param_1 + grand_fsu._16_8_;
  }
  cVar1 = known_value(param_1[3]);
  if (cVar1 != '\0') {
    add_uint_with_neg_flag(0x104378,0x104380,param_1[3] * *param_1,*(undefined *)(param_1 + 4));
  }
  return;
}

// Function: get_dev
void get_dev(undefined *param_1,char *param_2,undefined *param_3,char *param_4,undefined *param_5,
            char param_6,char param_7,undefined8 *param_8,char param_9) {
  uint uVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  undefined8 uVar6;
  undefined8 *puVar7;
  void *pvVar8;
  ulong uVar9;
  long in_FS_OFFSET;
  double dVar10;
  double dVar11;
  undefined *local_4c0;
  char *local_4b8;
  undefined *local_4b0;
  undefined *local_4a0;
  void *local_480;
  long local_478;
  long local_470;
  undefined8 *local_468;
  double local_460;
  undefined8 local_408;
  long local_400;
  undefined8 local_3f8;
  undefined8 local_3f0;
  ulong local_3e8;
  undefined8 local_3e0;
  undefined8 local_3d8;
  undefined8 local_3c8 [8];
  undefined8 local_388 [8];
  stat local_348;
  undefined local_2b8 [664];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (((((param_7 != '\0') && (show_local_fs != '\0')) ||
       ((param_6 != '\0' && ((show_all_fs != '\x01' && (show_listed_fs != '\x01')))))) ||
      (cVar2 = selected_fstype(param_5), cVar2 != '\x01')) ||
     ((cVar2 = excluded_fstype(param_5), cVar2 != '\0' ||
      ((param_8 == (undefined8 *)0x0 && (*param_2 != '/')))))) goto LAB_00102942;
  local_4b8 = param_4;
  if (param_4 == (char *)0x0) {
    local_4b8 = param_2;
  }
  local_4c0 = param_5;
  if (param_8 == (undefined8 *)0x0) {
    iVar3 = get_fs_usage(local_4b8,param_1,&local_408);
    if (iVar3 == 0) {
      if (((((param_9 != '\0') && (show_all_fs != '\0')) &&
           (iVar3 = stat(local_4b8,&local_348), iVar3 == 0)) &&
          ((puVar7 = (undefined8 *)me_for_dev(local_348.st_dev), puVar7 != (undefined8 *)0x0 &&
           (cVar2 = streq(*puVar7,param_1), cVar2 != '\x01')))) &&
         (((*(byte *)(puVar7 + 5) & 2) == 0 || (param_7 != '\x01')))) {
        local_4c0 = &DAT_001049c1;
        local_3e8 = local_3e8 & 0xffffffffffffff00;
        local_3d8 = 0xffffffffffffffff;
        local_3e0 = 0xffffffffffffffff;
        local_3f0 = 0xffffffffffffffff;
        local_3f8 = 0xffffffffffffffff;
        local_400 = -1;
        local_408 = 0xffffffffffffffff;
      }
    }
    else {
      if ((param_9 == '\0') ||
         ((piVar5 = __errno_location(), *piVar5 != 0xd &&
          (piVar5 = __errno_location(), *piVar5 != 2)))) {
        uVar6 = quotearg_n_style_colon(0,3,local_4b8);
        piVar5 = __errno_location();
        error(0,*piVar5,&DAT_001049db,uVar6);
        exit_status = 1;
        goto LAB_00102942;
      }
      if (show_all_fs != '\x01') goto LAB_00102942;
      local_4c0 = &DAT_001049c1;
      local_3e8 = local_3e8 & 0xffffffffffffff00;
      local_3d8 = 0xffffffffffffffff;
      local_3e0 = 0xffffffffffffffff;
      local_3f0 = 0xffffffffffffffff;
      local_3f8 = 0xffffffffffffffff;
      local_400 = -1;
      local_408 = 0xffffffffffffffff;
    }
  }
  else {
    local_408 = *param_8;
    local_400 = param_8[1];
    local_3f8 = param_8[2];
    local_3f0 = param_8[3];
    local_3e8 = param_8[4];
    local_3e0 = param_8[5];
    local_3d8 = param_8[6];
  }
  if (((local_400 != 0) || (show_all_fs == '\x01')) || (show_listed_fs == '\x01')) {
    if (param_8 == (undefined8 *)0x0) {
      file_systems_processed = 1;
    }
    alloc_table_row();
    local_4a0 = param_1;
    if (param_1 == (undefined *)0x0) {
      local_4a0 = &DAT_001049c1;
    }
    local_4b0 = param_3;
    if (param_3 == (undefined *)0x0) {
      local_4b0 = &DAT_001049c1;
    }
    local_480 = (void *)xstrdup(local_4a0);
    if (((param_9 != '\0') && (cVar2 = has_uuid_suffix(local_480), cVar2 != '\0')) &&
       (pvVar8 = (void *)canonicalize_filename_mode(local_480,0), pvVar8 != (void *)0x0)) {
      free(local_480);
      local_480 = pvVar8;
    }
    if (local_4c0 == (undefined *)0x0) {
      local_4c0 = &DAT_001049c1;
    }
    get_field_values(local_3c8,local_388,&local_408);
    if ((print_grand_total != '\0') && (param_8 == (undefined8 *)0x0)) {
      add_to_grand_total(local_3c8,local_388);
    }
    for (local_478 = 0; local_478 < ncolumns; local_478 = local_478 + 1) {
      uVar1 = *(uint *)(*(long *)(columns + local_478 * 8) + 0x10);
      if (uVar1 == 2) {
        local_468 = (undefined8 *)0x0;
      }
      else {
        if (2 < uVar1) {
LAB_00102249:
                    /* WARNING: Subroutine does not return */
          __assert_fail("!\"bad field_type\"",
                        "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/df.c"
                        ,0x46b,"get_dev");
        }
        if (uVar1 == 0) {
          local_468 = local_3c8;
        }
        else {
          if (uVar1 != 1) goto LAB_00102249;
          local_468 = local_388;
        }
      }
      switch(**(undefined4 **)(columns + local_478 * 8)) {
      case 0:
        local_470 = xstrdup(local_480);
        break;
      case 1:
        local_470 = xstrdup(local_4c0);
        break;
      case 2:
      case 6:
        uVar6 = df_readable(0,local_468[2],local_2b8,*local_468,local_468[1]);
        local_470 = xstrdup(uVar6);
        break;
      case 3:
      case 7:
        uVar6 = df_readable(*(undefined *)(local_468 + 7),local_468[6],local_2b8,*local_468,
                            local_468[1]);
        local_470 = xstrdup(uVar6);
        break;
      case 4:
      case 8:
        uVar6 = df_readable(*(undefined *)(local_468 + 4),local_468[3],local_2b8,*local_468,
                            local_468[1]);
        local_470 = xstrdup(uVar6);
        break;
      case 5:
      case 9:
        local_460 = DAT_00105440;
        cVar2 = known_value(local_468[6]);
        if ((cVar2 == '\x01') && (cVar2 = known_value(local_468[3]), cVar2 == '\x01')) {
          if ((*(char *)(local_468 + 7) == '\x01') ||
             (((0x28f5c28f5c28f5c < (ulong)local_468[6] || (local_468[3] + local_468[6] == 0)) ||
              ((ulong)(local_468[6] + local_468[3]) < (ulong)local_468[6] !=
               (bool)*(char *)(local_468 + 4))))) {
            if (*(char *)(local_468 + 7) == '\0') {
              uVar9 = local_468[6];
              if ((long)uVar9 < 0) {
                dVar10 = (double)(uVar9 >> 1 | (ulong)((uint)uVar9 & 1));
                dVar10 = dVar10 + dVar10;
              }
              else {
                dVar10 = (double)uVar9;
              }
            }
            else {
              uVar9 = -local_468[6];
              if ((long)local_468[6] < 1) {
                dVar10 = (double)uVar9;
              }
              else {
                dVar10 = (double)(uVar9 >> 1 | (ulong)((uint)uVar9 & 1));
                dVar10 = dVar10 + dVar10;
              }
              dVar10 = (double)((ulong)dVar10 ^ DAT_00105450);
            }
            if (*(char *)(local_468 + 4) == '\0') {
              uVar9 = local_468[3];
              if ((long)uVar9 < 0) {
                dVar11 = (double)(uVar9 >> 1 | (ulong)((uint)uVar9 & 1));
                dVar11 = dVar11 + dVar11;
              }
              else {
                dVar11 = (double)uVar9;
              }
            }
            else {
              uVar9 = -local_468[3];
              if ((long)local_468[3] < 1) {
                dVar11 = (double)uVar9;
              }
              else {
                dVar11 = (double)(uVar9 >> 1 | (ulong)((uint)uVar9 & 1));
                dVar11 = dVar11 + dVar11;
              }
              dVar11 = (double)((ulong)dVar11 ^ DAT_00105450);
            }
            if (dVar10 + dVar11 != 0.0) {
              local_460 = (DAT_00105460 * dVar10) / (dVar10 + dVar11);
              dVar10 = (double)(long)local_460;
              if ((dVar10 - DAT_00105468 < local_460) && (local_460 <= DAT_00105468 + dVar10)) {
                dVar11 = DAT_00105468;
                if (local_460 <= dVar10) {
                  dVar11 = 0.0;
                }
                local_460 = dVar11 + dVar10;
              }
            }
          }
          else {
            uVar9 = (ulong)((ulong)(local_468[6] * 100) % (ulong)(local_468[3] + local_468[6]) != 0)
                    + (ulong)(local_468[6] * 100) / (ulong)(local_468[3] + local_468[6]);
            if ((long)uVar9 < 0) {
              local_460 = (double)(uVar9 >> 1 | (ulong)((uint)uVar9 & 1));
              local_460 = local_460 + local_460;
            }
            else {
              local_460 = (double)uVar9;
            }
          }
        }
        if (0.0 <= local_460) {
          local_470 = xasprintf(local_460,"%.0f%%");
        }
        else {
          local_470 = xstrdup(&DAT_001049c1);
        }
        break;
      case 10:
        local_470 = xstrdup(param_2);
        break;
      case 0xb:
        local_470 = xstrdup(local_4b0);
        break;
      default:
                    /* WARNING: Subroutine does not return */
        __assert_fail("!\"unhandled field\"",
                      "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/df.c",
                      0x4c8,"get_dev");
      }
      if (local_470 == 0) {
                    /* WARNING: Subroutine does not return */
        __assert_fail("cell",
                      "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/df.c",
                      0x4cb,"get_dev");
      }
      replace_problematic_chars(local_470);
      iVar4 = gnu_mbswidth(local_470,3);
      iVar3 = *(int *)(*(long *)(columns + local_478 * 8) + 0x20);
      if (iVar4 <= iVar3) {
        iVar4 = iVar3;
      }
      *(int *)(*(long *)(columns + local_478 * 8) + 0x20) = iVar4;
      *(long *)(local_478 * 8 + *(long *)(table + nrows * 8 + -8)) = local_470;
    }
    free(local_480);
  }
LAB_00102942:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: last_device_for_mount
char * last_device_for_mount(undefined8 param_1) {
  char cVar1;
  char *__ptr;
  char **local_28;
  char **local_20;
  
  local_20 = (char **)0x0;
  for (local_28 = mount_list; local_28 != (char **)0x0; local_28 = (char **)local_28[6]) {
    cVar1 = streq(local_28[1],param_1);
    if (cVar1 != '\0') {
      local_20 = local_28;
    }
  }
  if (local_20 == (char **)0x0) {
    __ptr = (char *)0x0;
  }
  else {
    __ptr = canonicalize_file_name(*local_20);
    if ((__ptr == (char *)0x0) || (*__ptr != '/')) {
      free(__ptr);
      __ptr = (char *)xstrdup(*local_20);
    }
  }
  return __ptr;
}

// Function: get_device
undefined8 get_device(char *param_1) {
  bool bVar1;
  bool bVar2;
  char cVar3;
  int iVar4;
  char *__ptr;
  char *__ptr_00;
  void *__ptr_01;
  size_t sVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  long in_FS_OFFSET;
  char *local_120;
  char **local_108;
  char **local_100;
  ulong local_f8;
  char *local_f0;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_100 = (char **)0x0;
  bVar2 = false;
  bVar1 = false;
  __ptr = canonicalize_file_name(param_1);
  local_120 = param_1;
  if ((__ptr != (char *)0x0) && (local_120 = param_1, *__ptr == '/')) {
    local_120 = __ptr;
  }
  local_f8 = 0xffffffffffffffff;
  local_108 = mount_list;
  do {
    if (local_108 == (char **)0x0) {
LAB_00102cbf:
      free(__ptr);
      if (local_100 == (char **)0x0) {
        if (bVar1) {
          uVar6 = quotearg_style(4,param_1);
          uVar7 = gettext("cannot access %s: over-mounted by another device");
          error(0,0,uVar7,uVar6);
          exit_status = 1;
          uVar6 = 1;
        }
        else {
          uVar6 = 0;
        }
      }
      else {
        get_dev(*local_100,local_100[1],param_1,0,local_100[3],(*(byte *)(local_100 + 5) & 1) != 0,
                (*(byte *)(local_100 + 5) & 2) != 0,0,0);
        uVar6 = 1;
      }
      if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return uVar6;
    }
    local_f0 = *local_108;
    __ptr_00 = canonicalize_file_name(*local_108);
    if ((__ptr_00 != (char *)0x0) && (*__ptr_00 == '/')) {
      local_f0 = __ptr_00;
    }
    cVar3 = streq(local_120,local_f0);
    if (cVar3 != '\0') {
      __ptr_01 = (void *)last_device_for_mount(local_108[1]);
      if ((__ptr_01 == (void *)0x0) || (cVar3 = streq(__ptr_01,local_f0), cVar3 == '\x01')) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      sVar5 = strlen(local_108[1]);
      if ((!bVar1) && ((!bVar2 || (sVar5 < local_f8)))) {
        iVar4 = stat(local_108[1],&local_b8);
        if (iVar4 == 0) {
          bVar2 = true;
        }
        else if ((bVar2) || (local_f8 <= sVar5)) goto LAB_00102c81;
        local_100 = local_108;
        local_f8 = sVar5;
        if (sVar5 == 1) {
          free(__ptr_01);
          free(__ptr_00);
          goto LAB_00102cbf;
        }
      }
LAB_00102c81:
      free(__ptr_01);
    }
    free(__ptr_00);
    local_108 = (char **)local_108[6];
  } while( true );
}

// Function: get_point
void get_point(char *param_1,__dev_t *param_2) {
  char cVar1;
  int iVar2;
  char *__s;
  size_t sVar3;
  size_t __n;
  int *piVar4;
  undefined8 uVar5;
  void *__ptr;
  long in_FS_OFFSET;
  undefined8 *local_f8;
  undefined8 *local_f0;
  ulong local_e8;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_f0 = (undefined8 *)0x0;
  __s = canonicalize_file_name(param_1);
  if ((__s != (char *)0x0) && (*__s == '/')) {
    sVar3 = strlen(__s);
    local_e8 = 0;
    for (local_f8 = mount_list; local_f8 != (undefined8 *)0x0; local_f8 = (undefined8 *)local_f8[6])
    {
      cVar1 = streq(local_f8[3],&DAT_00104a71);
      if ((cVar1 != '\x01') &&
         (((local_f0 == (undefined8 *)0x0 || ((*(byte *)(local_f0 + 5) & 1) != 0)) ||
          ((*(byte *)(local_f8 + 5) & 1) == 0)))) {
        __n = strlen((char *)local_f8[1]);
        if ((local_e8 <= __n) && (__n <= sVar3)) {
          if (__n == 1) {
LAB_00102f87:
            local_f0 = local_f8;
            local_e8 = __n;
          }
          else if ((__n == sVar3) || (__s[__n] == '/')) {
            iVar2 = strncmp((char *)local_f8[1],__s,__n);
            if (iVar2 == 0) goto LAB_00102f87;
          }
        }
      }
    }
  }
  free(__s);
  if (local_f0 != (undefined8 *)0x0) {
    iVar2 = stat((char *)local_f0[1],&local_b8);
    if ((iVar2 != 0) || (local_b8.st_dev != *param_2)) {
      local_f0 = (undefined8 *)0x0;
    }
  }
  if (local_f0 == (undefined8 *)0x0) {
    for (local_f8 = mount_list; local_f8 != (undefined8 *)0x0; local_f8 = (undefined8 *)local_f8[6])
    {
      if (local_f8[4] == -1) {
        iVar2 = stat((char *)local_f8[1],&local_b8);
        if (iVar2 == 0) {
          local_f8[4] = local_b8.st_dev;
        }
        else {
          piVar4 = __errno_location();
          if (*piVar4 == 5) {
            uVar5 = quotearg_n_style_colon(0,3,local_f8[1]);
            piVar4 = __errno_location();
            error(0,*piVar4,&DAT_001049db,uVar5);
            exit_status = 1;
          }
          local_f8[4] = 0xfffffffffffffffe;
        }
      }
      if (*param_2 == local_f8[4]) {
        cVar1 = streq(local_f8[3],&DAT_00104a71);
        if ((cVar1 != '\x01') &&
           (((local_f0 == (undefined8 *)0x0 || ((*(byte *)(local_f0 + 5) & 1) != 0)) ||
            ((*(byte *)(local_f8 + 5) & 1) == 0)))) {
          iVar2 = stat((char *)local_f8[1],&local_b8);
          if ((iVar2 == 0) && (local_b8.st_dev == local_f8[4])) {
            local_f0 = local_f8;
          }
          else {
            local_f8[4] = 0xfffffffffffffffe;
          }
        }
      }
    }
  }
  if (local_f0 == (undefined8 *)0x0) {
    __ptr = (void *)find_mount_point(param_1,param_2);
    if (__ptr != (void *)0x0) {
      get_dev(0,__ptr,param_1,0,0,0,0,0,0);
      free(__ptr);
    }
  }
  else {
    get_dev(*local_f0,local_f0[1],param_1,param_1,local_f0[3],(*(byte *)(local_f0 + 5) & 1) != 0,
            (*(byte *)(local_f0 + 5) & 2) != 0,0,0);
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: get_entry
void get_entry(undefined8 param_1,long param_2) {
  char cVar1;
  
  if ((((*(uint *)(param_2 + 0x18) & 0xf000) == 0x6000) ||
      ((*(uint *)(param_2 + 0x18) & 0xf000) == 0x2000)) &&
     (cVar1 = get_device(param_1), cVar1 != '\0')) {
    return;
  }
  get_point(param_1,param_2);
  return;
}

// Function: get_all_entries
void get_all_entries(void) {
  undefined8 *local_10;
  
  filter_mount_list(show_all_fs);
  for (local_10 = mount_list; local_10 != (undefined8 *)0x0; local_10 = (undefined8 *)local_10[6]) {
    get_dev(*local_10,local_10[1],0,0,local_10[3],(*(byte *)(local_10 + 5) & 1) != 0,
            (*(byte *)(local_10 + 5) & 2) != 0,0,1);
  }
  return;
}

// Function: add_fs_type
void add_fs_type(undefined8 param_1) {
  undefined8 *puVar1;
  
  puVar1 = (undefined8 *)xmalloc(0x10);
  *puVar1 = param_1;
  puVar1[1] = fs_select_list;
  fs_select_list = puVar1;
  return;
}

// Function: add_excluded_fs_type
void add_excluded_fs_type(undefined8 param_1) {
  undefined8 *puVar1;
  
  puVar1 = (undefined8 *)xmalloc(0x10);
  *puVar1 = param_1;
  puVar1[1] = fs_exclude_list;
  fs_exclude_list = puVar1;
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
    pcVar3 = (char *)gettext(
                            "Show information about the file system on which each FILE resides,\nor all file systems by default.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -a, --all             include pseudo, duplicate, inaccessible file systems\n  -B, --block-size=SIZE  scale sizes by SIZE before printing them; e.g.,\n                           \'-BM\' prints sizes in units of 1,048,576 bytes;\n                           see SIZE format below\n  -h, --human-readable  print sizes in powers of 1024 (e.g., 1023M)\n  -H, --si              print sizes in powers of 1000 (e.g., 1.1G)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -i, --inodes          list inode information instead of block usage\n  -k                    like --block-size=1K\n  -l, --local           limit listing to local file systems\n      --no-sync         do not invoke sync before getting usage info (default)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --output[=FIELD_LIST]  use the output format defined by FIELD_LIST,\n                               or print all fields if FIELD_LIST is omitted\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -P, --portability     use the POSIX output format\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --sync            invoke sync before getting usage info\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --total           elide all entries insignificant to available space,\n                          and produce a grand total\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -t, --type=TYPE       limit listing to file systems of type TYPE\n  -T, --print-type      print file system type\n  -x, --exclude-type=TYPE   limit listing to file systems not of type TYPE\n  -v                    (ignored)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_blocksize_note(&DAT_001050b1);
    emit_size_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nFIELD_LIST is a comma-separated list of columns to be included.  Valid\nfield names are: \'source\', \'fstype\', \'itotal\', \'iused\', \'iavail\', \'ipcent\',\n\'size\', \'used\', \'avail\', \'pcent\', \'file\' and \'target\' (see info page).\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_00105194);
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
  bool bVar1;
  char cVar2;
  undefined4 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  int *piVar7;
  char *pcVar8;
  long in_FS_OFFSET;
  undefined4 local_108;
  int local_104;
  int local_100;
  int local_fc;
  int local_f8;
  int local_f4;
  int local_f0;
  int local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  int local_ac;
  int local_a8;
  int local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  long local_98;
  undefined8 *local_90;
  undefined8 *local_88;
  undefined8 local_80;
  undefined *local_78;
  undefined8 local_70;
  undefined8 local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined *local_38;
  undefined *local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_98 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  fs_select_list = (undefined8 *)0x0;
  fs_exclude_list = (undefined8 *)0x0;
  show_all_fs = '\0';
  show_listed_fs = 0;
  human_output_opts = 0xffffffff;
  print_type = '\0';
  file_systems_processed = '\0';
  exit_status = 0;
  print_grand_total = '\0';
  grand_fsu._0_8_ = 1;
  bVar1 = false;
  local_80 = gettext("options %s and %s are mutually exclusive");
LAB_001037b8:
  local_108 = 0xffffffff;
  local_f8 = getopt_long(param_1,param_2,"aB:iF:hHklmPTt:vx:",long_options,&local_108);
  if (local_f8 == -1) {
    if (human_output_opts == 0xffffffff) {
      if (bVar1) {
        human_output_opts = 0;
        pcVar8 = getenv("POSIXLY_CORRECT");
        if (pcVar8 == (char *)0x0) {
          output_block_size = 0x400;
        }
        else {
          output_block_size = 0x200;
        }
      }
      else {
        pcVar8 = getenv("DF_BLOCK_SIZE");
        human_options(pcVar8,&human_output_opts,&output_block_size);
      }
    }
    if ((header_mode != 1) && (header_mode != 4)) {
      if ((human_output_opts & 0x10) == 0) {
        if (bVar1) {
          header_mode = 3;
        }
      }
      else {
        header_mode = 2;
      }
    }
    bVar1 = false;
    local_90 = fs_select_list;
    do {
      if (local_90 == (undefined8 *)0x0) {
        if (!bVar1) {
          if (_optind < param_1) {
            local_98 = xnmalloc((long)(param_1 - _optind),0x90);
            for (local_104 = _optind; local_104 < param_1; local_104 = local_104 + 1) {
              local_f0 = automount_stat_err(param_2[local_104],
                                            (long)(local_104 - _optind) * 0x90 + local_98);
              if (local_f0 != 0) {
                uVar6 = quotearg_n_style_colon(0,3,param_2[local_104]);
                local_b0 = 0;
                local_ac = local_f0;
                local_38 = &DAT_001049db;
                error(0,local_f0,&DAT_001049db,uVar6);
                exit_status = 1;
                param_2[local_104] = 0;
              }
            }
          }
          if ((((fs_select_list == (undefined8 *)0x0) && (fs_exclude_list == (undefined8 *)0x0)) &&
              (print_type == '\0')) && ((field_data[77] == '\0' && (show_local_fs == '\0')))) {
            uVar3 = 0;
          }
          else {
            uVar3 = 1;
          }
          mount_list = read_file_system_list(uVar3);
          if (mount_list == 0) {
            local_100 = 0;
            if (((param_1 <= _optind) || (show_all_fs != '\0')) ||
               ((show_local_fs != '\0' ||
                ((fs_select_list != (undefined8 *)0x0 || (fs_exclude_list != (undefined8 *)0x0))))))
            {
              local_100 = 1;
            }
            if (local_100 == 0) {
              local_78 = (undefined *)gettext("Warning: ");
            }
            else {
              local_78 = &DAT_001046e4;
            }
            local_ec = local_100;
            uVar6 = gettext("cannot read table of mounted file systems");
            piVar7 = __errno_location();
            local_a4 = *piVar7;
            local_a8 = local_ec;
            local_30 = &DAT_001052da;
            error(local_ec,local_a4,&DAT_001052da,local_78,uVar6);
          }
          if (require_sync != '\0') {
            sync();
          }
          get_field_list();
          get_header();
          if (local_98 == 0) {
            get_all_entries();
          }
          else {
            show_listed_fs = 1;
            for (local_fc = _optind; local_fc < param_1; local_fc = local_fc + 1) {
              if (param_2[local_fc] != 0) {
                get_entry(param_2[local_fc],(long)(local_fc - _optind) * 0x90 + local_98);
              }
            }
          }
          if (file_systems_processed == '\0') {
            if (exit_status == 0) {
              local_28 = gettext("no file systems processed");
              local_a0 = 1;
              local_9c = 0;
              error(1,0,local_28);
            }
          }
          else {
            if (print_grand_total != '\0') {
              if (field_data[37] == '\0') {
                pcVar8 = "total";
              }
              else {
                pcVar8 = "-";
              }
              get_dev("total",pcVar8,0,0,0,0,0,grand_fsu,0);
            }
            print_table();
          }
                    /* WARNING: Subroutine does not return */
          exit(exit_status);
        }
        if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        return 1;
      }
      for (local_88 = fs_exclude_list; local_88 != (undefined8 *)0x0;
          local_88 = (undefined8 *)local_88[1]) {
        cVar2 = streq(*local_90,*local_88);
        if (cVar2 != '\0') {
          uVar6 = quote(*local_90);
          local_40 = gettext("file system type %s both selected and excluded");
          local_b8 = 0;
          local_b4 = 0;
          error(0,0,local_40,uVar6);
          bVar1 = true;
          break;
        }
      }
      local_90 = (undefined8 *)local_90[1];
    } while( true );
  }
  if (local_f8 < 0x84) {
    if (local_f8 < 0x42) {
      if (local_f8 == -0x83) {
LAB_00103c42:
        uVar6 = proper_name_lite("Paul Eggert","Paul Eggert");
        uVar4 = proper_name_lite("David MacKenzie","David MacKenzie");
        uVar5 = proper_name_lite("Torbjorn Granlund",&DAT_0010521a);
        version_etc(_stdout,&DAT_00105194,"GNU coreutils",_Version,uVar5,uVar4,uVar6,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (local_f8 == -0x82) {
        usage(0);
        goto LAB_00103c42;
      }
      goto switchD_00103884_caseD_43;
    }
    goto code_r0x00103864;
  }
  goto switchD_00103884_caseD_43;
code_r0x00103864:
  switch(local_f8) {
  case 0x42:
    local_f4 = human_options(_optarg,&human_output_opts,&output_block_size);
    if (local_f4 == 0) break;
    xstrtol_fatal(local_f4,local_108,(int)(char)local_f8,long_options,_optarg);
  case 0x69:
    if (header_mode == 4) {
      local_e8 = 0;
      local_e4 = 0;
      local_70 = local_80;
      error(0,0,local_80,&DAT_001051f5,"--output");
      usage(1);
    }
    header_mode = 1;
    break;
  default:
switchD_00103884_caseD_43:
    usage(1);
    break;
  case 0x46:
  case 0x74:
    add_fs_type(_optarg);
    break;
  case 0x48:
    human_output_opts = 0x90;
    output_block_size = 1;
    break;
  case 0x50:
    if (header_mode == 4) {
      local_d8 = 0;
      local_d4 = 0;
      local_60 = local_80;
      error(0,0,local_80,&DAT_001051fb,"--output");
      usage(1);
    }
    bVar1 = true;
    break;
  case 0x54:
    if (header_mode == 4) {
      local_e0 = 0;
      local_dc = 0;
      local_68 = local_80;
      error(0,0,local_80,&DAT_001051f8,"--output");
      usage(1);
    }
    print_type = '\x01';
    break;
  case 0x61:
    show_all_fs = '\x01';
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
    show_local_fs = '\x01';
    break;
  case 0x6d:
    human_output_opts = 0;
    output_block_size = 0x100000;
    break;
  case 0x76:
    break;
  case 0x78:
    add_excluded_fs_type(_optarg);
    break;
  case 0x80:
    require_sync = '\0';
    break;
  case 0x81:
    require_sync = '\x01';
    break;
  case 0x82:
    print_grand_total = '\x01';
    break;
  case 0x83:
    if (header_mode == 1) {
      local_d0 = 0;
      local_cc = 0;
      local_58 = local_80;
      error(0,0,local_80,&DAT_001051f5,"--output");
      usage(1);
    }
    if ((bVar1) && (header_mode == 0)) {
      local_c8 = 0;
      local_c4 = 0;
      local_50 = local_80;
      error(0,0,local_80,&DAT_001051fb,"--output");
      usage(1);
    }
    if (print_type != '\0') {
      local_c0 = 0;
      local_bc = 0;
      local_48 = local_80;
      error(0,0,local_80,&DAT_001051f8,"--output");
      usage(1);
    }
    header_mode = 4;
    if (_optarg != 0) {
      decode_output_arg(_optarg);
    }
  }
  goto LAB_001037b8;
}

