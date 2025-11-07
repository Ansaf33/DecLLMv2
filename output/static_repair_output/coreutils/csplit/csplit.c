// Function: cleanup
void cleanup(EVP_PKEY_CTX *ctx) {
  long in_FS_OFFSET;
  sigset_t local_98;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  close_output_file();
  sigprocmask(0,(sigset_t *)&caught_signals,&local_98);
  delete_all_files(0);
  sigprocmask(2,&local_98,(sigset_t *)0x0);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: cleanup_fatal
void cleanup_fatal(EVP_PKEY_CTX *param_1) {
  cleanup(param_1);
                    /* WARNING: Subroutine does not return */
  exit(1);
}

// Function: xalloc_die
void xalloc_die(void) {
  undefined8 uVar1;
  
  uVar1 = gettext("memory exhausted");
  error(0,0,&DAT_00103500,uVar1);
                    /* WARNING: Subroutine does not return */
  cleanup_fatal();
}

// Function: interrupt_handler
void interrupt_handler(int param_1) {
  delete_all_files(1);
  signal(param_1,(__sighandler_t)0x0);
  raise(param_1);
  return;
}

// Function: save_to_hold_area
void save_to_hold_area(undefined8 param_1,undefined8 param_2) {
  free(hold_area);
  hold_area = (void *)param_1;
  hold_count = param_2;
  return;
}

// Function: read_input
long read_input(undefined8 param_1,long param_2) {
  long lVar1;
  undefined8 uVar2;
  int *piVar3;
  
  if (param_2 == 0) {
    lVar1 = 0;
  }
  else {
    lVar1 = safe_read(0,param_1,param_2);
    if (lVar1 == 0) {
      have_read_eof = 1;
    }
    if (lVar1 < 0) {
      uVar2 = gettext("read error");
      piVar3 = __errno_location();
      error(0,*piVar3,uVar2);
                    /* WARNING: Subroutine does not return */
      cleanup_fatal();
    }
  }
  return lVar1;
}

// Function: clear_line_control
void clear_line_control(undefined8 *param_1) {
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  return;
}

// Function: new_line_control
long new_line_control(void) {
  long lVar1;
  
  lVar1 = xmalloc(0x520);
  *(undefined8 *)(lVar1 + 0x518) = 0;
  clear_line_control(lVar1);
  return lVar1;
}

// Function: keep_new_line
void keep_new_line(long param_1,long param_2,long param_3) {
  long lVar1;
  long *plVar2;
  undefined8 uVar3;
  
  if (*(long *)(param_1 + 0x30) == 0) {
    uVar3 = new_line_control();
    *(undefined8 *)(param_1 + 0x38) = uVar3;
    *(undefined8 *)(param_1 + 0x30) = *(undefined8 *)(param_1 + 0x38);
  }
  if (**(long **)(param_1 + 0x38) == 0x50) {
    lVar1 = *(long *)(param_1 + 0x38);
    uVar3 = new_line_control();
    *(undefined8 *)(lVar1 + 0x518) = uVar3;
    *(undefined8 *)(param_1 + 0x38) = *(undefined8 *)(*(long *)(param_1 + 0x38) + 0x518);
  }
  plVar2 = *(long **)(param_1 + 0x38);
  plVar2[(plVar2[1] + 1) * 2 + 2] = param_2;
  plVar2[(plVar2[1] + 1) * 2 + 1] = param_3;
  *plVar2 = *plVar2 + 1;
  plVar2[1] = plVar2[1] + 1;
  return;
}

// Function: record_line_starts
long record_line_starts(long param_1) {
  undefined *puVar1;
  undefined *puVar2;
  long lVar3;
  undefined8 uVar4;
  undefined *local_38;
  long local_30;
  
  if (*(long *)(param_1 + 8) == 0) {
    local_30 = 0;
  }
  else {
    local_30 = 0;
    local_38 = *(undefined **)(param_1 + 0x28);
    puVar1 = local_38 + *(long *)(param_1 + 8);
    *puVar1 = 10;
    while( true ) {
      puVar2 = (undefined *)rawmemchr(local_38,10);
      if (puVar2 == puVar1) break;
      keep_new_line(param_1,local_38,puVar2 + (1 - (long)local_38));
      local_38 = puVar2 + 1;
      local_30 = local_30 + 1;
    }
    lVar3 = (long)puVar1 - (long)local_38;
    if (lVar3 != 0) {
      if (have_read_eof == '\0') {
        uVar4 = ximemdup(local_38,lVar3);
        save_to_hold_area(uVar4,lVar3);
      }
      else {
        keep_new_line(param_1,local_38,lVar3);
        local_30 = local_30 + 1;
      }
    }
    *(long *)(param_1 + 0x20) = local_30;
    *(long *)(param_1 + 0x10) = last_line_number + 1;
    *(undefined8 *)(param_1 + 0x18) = *(undefined8 *)(param_1 + 0x10);
    last_line_number = local_30 + last_line_number;
  }
  return local_30;
}

// Function: free_buffer
void free_buffer(void *param_1) {
  void *pvVar1;
  void *local_18;
  
  local_18 = *(void **)((long)param_1 + 0x30);
  while (local_18 != (void *)0x0) {
    pvVar1 = *(void **)((long)local_18 + 0x518);
    free(local_18);
    local_18 = pvVar1;
  }
  free(*(void **)((long)param_1 + 0x28));
  free(param_1);
  return;
}

// Function: save_buffer
void save_buffer(long param_1) {
  long lVar1;
  long local_10;
  
  *(undefined8 *)(param_1 + 0x40) = 0;
  *(undefined8 *)(param_1 + 0x38) = *(undefined8 *)(param_1 + 0x30);
  lVar1 = param_1;
  if (head != 0) {
    for (local_10 = head; *(long *)(local_10 + 0x40) != 0; local_10 = *(long *)(local_10 + 0x40)) {
    }
    *(long *)(local_10 + 0x40) = param_1;
    lVar1 = head;
  }
  head = lVar1;
  return;
}

// Function: load_buffer
undefined8 load_buffer(void) {
  long lVar1;
  long lVar2;
  undefined8 uVar3;
  size_t sVar4;
  long *plVar5;
  long lVar6;
  long in_FS_OFFSET;
  long local_38;
  long local_30;
  void *local_28;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  if (have_read_eof == '\0') {
    sVar4 = hold_count;
    if ((long)hold_count < 0x1ffe) {
      sVar4 = 0x1ffe;
    }
    local_38 = sVar4 + 1;
    do {
      plVar5 = (long *)get_new_buffer(local_38);
      lVar2 = *plVar5;
      local_28 = (void *)plVar5[5];
      local_30 = lVar2;
      if (hold_count != 0) {
        local_28 = mempcpy(local_28,hold_area,hold_count);
        plVar5[1] = plVar5[1] + hold_count;
        local_30 = lVar2 - hold_count;
        hold_count = 0;
      }
      lVar6 = read_input(local_28,local_30 + -1);
      plVar5[1] = plVar5[1] + lVar6;
      lVar6 = record_line_starts(plVar5);
      if (lVar6 != 0) {
        save_buffer(plVar5);
        uVar3 = 1;
        goto LAB_00100a6c;
      }
      free_buffer(plVar5);
      if (have_read_eof != '\0') {
        uVar3 = 0;
        goto LAB_00100a6c;
      }
      local_38 = lVar2 + (lVar2 >> 1);
    } while (!SCARRY8(lVar2,lVar2 >> 1));
    uVar3 = xalloc_die();
  }
  else {
    uVar3 = 0;
  }
LAB_00100a6c:
  if (lVar1 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: get_first_line_in_buffer
undefined8 get_first_line_in_buffer(void) {
  char cVar1;
  
  if ((head == 0) && (cVar1 = load_buffer(), cVar1 != '\x01')) {
    return 0;
  }
  return *(undefined8 *)(head + 0x18);
}

// Function: remove_line
long * remove_line(void) {
  long *plVar1;
  char cVar2;
  long *plVar3;
  
  if (prev_buf_3 != 0) {
    free_buffer(prev_buf_3);
    prev_buf_3 = 0;
  }
  if ((head == 0) && (cVar2 = load_buffer(), cVar2 != '\x01')) {
    plVar3 = (long *)0x0;
  }
  else {
    if (current_line < *(long *)(head + 0x18)) {
      current_line = *(long *)(head + 0x18);
    }
    *(long *)(head + 0x18) = *(long *)(head + 0x18) + 1;
    plVar1 = *(long **)(head + 0x38);
    plVar3 = plVar1 + (plVar1[2] + 1) * 2 + 1;
    plVar1[2] = plVar1[2] + 1;
    if ((plVar1[2] == *plVar1) &&
       ((*(long *)(head + 0x38) = plVar1[0xa3], *(long *)(head + 0x38) == 0 ||
        (**(long **)(head + 0x38) == 0)))) {
      prev_buf_3 = head;
      head = *(long *)(head + 0x40);
    }
  }
  return plVar3;
}

// Function: find_line
long find_line(long param_1) {
  char cVar1;
  long lVar2;
  long local_20;
  long local_18;
  long local_10;
  
  if ((head == 0) && (cVar1 = load_buffer(), cVar1 != '\x01')) {
    return 0;
  }
  if (param_1 < *(long *)(head + 0x10)) {
    lVar2 = 0;
  }
  else {
    for (local_20 = head; *(long *)(local_20 + 0x20) + *(long *)(local_20 + 0x10) <= param_1;
        local_20 = *(long *)(local_20 + 0x40)) {
      if ((*(long *)(local_20 + 0x40) == 0) && (cVar1 = load_buffer(), cVar1 != '\x01')) {
        return 0;
      }
    }
    local_18 = *(long *)(local_20 + 0x30);
    for (local_10 = param_1 - *(long *)(local_20 + 0x10); 0x4f < local_10;
        local_10 = local_10 + -0x50) {
      local_18 = *(long *)(local_18 + 0x518);
    }
    lVar2 = local_18 + (local_10 + 1) * 0x10 + 8;
  }
  return lVar2;
}

// Function: no_more_lines
bool no_more_lines(void) {
  long lVar1;
  
  lVar1 = find_line(current_line + 1);
  return lVar1 == 0;
}

// Function: set_input_file
void set_input_file(undefined8 param_1) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  int *piVar5;
  
  cVar1 = streq(param_1,&DAT_0010350e);
  if (cVar1 != '\x01') {
    iVar2 = fd_reopen(0,param_1,0,0);
    if (iVar2 < 0) {
      uVar3 = quotearg_style(4,param_1);
      uVar4 = gettext("cannot open %s for reading");
      piVar5 = __errno_location();
      error(1,*piVar5,uVar4,uVar3);
    }
  }
  return;
}

// Function: write_to_file
void write_to_file(long param_1,char param_2,int param_3) {
  long lVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  long lVar4;
  long local_48;
  
  lVar1 = get_first_line_in_buffer();
  if ((lVar1 != 0) && (lVar1 <= param_1)) {
    local_48 = 0;
    while( true ) {
      if (param_1 - lVar1 <= local_48) {
        return;
      }
      lVar4 = remove_line();
      if (lVar4 == 0) break;
      if (param_2 != '\x01') {
        save_line_to_file(lVar4);
      }
      local_48 = local_48 + 1;
    }
    uVar2 = quote(*(undefined8 *)(global_argv + (long)param_3 * 8));
    uVar3 = gettext("%s: line number out of range");
    error(0,0,uVar3,uVar2);
                    /* WARNING: Subroutine does not return */
    cleanup_fatal();
  }
  uVar2 = quote(*(undefined8 *)(global_argv + (long)param_3 * 8));
  uVar3 = gettext("%s: line number out of range");
  error(0,0,uVar3,uVar2);
                    /* WARNING: Subroutine does not return */
  cleanup_fatal();
}

// Function: dump_rest_of_file
void dump_rest_of_file(void) {
  long lVar1;
  
  while( true ) {
    lVar1 = remove_line();
    if (lVar1 == 0) break;
    save_line_to_file(lVar1);
  }
  return;
}

// Function: handle_line_error
void handle_line_error(long param_1,long param_2) {
  undefined8 uVar1;
  undefined8 uVar2;
  char *pcVar3;
  long in_FS_OFFSET;
  undefined local_38 [24];
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  uVar1 = imaxtostr(*(undefined8 *)(param_1 + 8),local_38);
  uVar2 = quote(uVar1);
  uVar1 = _program_name;
  pcVar3 = (char *)gettext("%s: %s: line number out of range");
  fprintf(_stderr,pcVar3,uVar1,uVar2);
  if (param_2 == 0) {
    fputc(10,_stderr);
  }
  else {
    pcVar3 = (char *)gettext(" on repetition %jd\n");
    fprintf(_stderr,pcVar3,param_2);
  }
                    /* WARNING: Subroutine does not return */
  cleanup_fatal();
}

// Function: process_line_count
void process_line_count(long param_1,long param_2) {
  long lVar1;
  char cVar2;
  long lVar3;
  long local_20;
  
  lVar1 = *(long *)(param_1 + 8);
  create_output_file();
  cVar2 = no_more_lines();
  if ((cVar2 != '\0') && (suppress_matched != '\0')) {
    handle_line_error(param_1,param_2);
  }
  local_20 = get_first_line_in_buffer();
  if (local_20 == 0) {
    handle_line_error(param_1,param_2);
  }
  while( true ) {
    if (lVar1 * (param_2 + 1) <= local_20) break;
    lVar3 = remove_line();
    if (lVar3 == 0) {
      handle_line_error(param_1,param_2);
    }
    save_line_to_file(lVar3);
    local_20 = local_20 + 1;
  }
  close_output_file();
  if (suppress_matched != '\0') {
    remove_line();
  }
  cVar2 = no_more_lines();
  if ((cVar2 != '\0') && (suppress_matched != '\x01')) {
    handle_line_error(param_1,param_2);
  }
  return;
}

// Function: regexp_error
void regexp_error(long param_1,long param_2,char param_3) {
  undefined8 uVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = quote(*(undefined8 *)((long)*(int *)(param_1 + 0x18) * 8 + global_argv));
  uVar1 = _program_name;
  pcVar3 = (char *)gettext("%s: %s: match not found");
  fprintf(_stderr,pcVar3,uVar1,uVar2);
  if (param_2 == 0) {
    fputc(10,_stderr);
  }
  else {
    pcVar3 = (char *)gettext(" on repetition %jd\n");
    fprintf(_stderr,pcVar3,param_2);
  }
  if (param_3 != '\x01') {
    dump_rest_of_file();
    close_output_file();
  }
                    /* WARNING: Subroutine does not return */
  cleanup_fatal();
}

// Function: process_regexp
void process_regexp(long *param_1,undefined8 param_2) {
  char cVar1;
  int iVar2;
  long *plVar3;
  undefined8 uVar4;
  long lVar5;
  long local_30;
  
  cVar1 = *(char *)((long)param_1 + 0x1d);
  if (cVar1 != '\x01') {
    create_output_file();
  }
  if (*param_1 < 0) {
    do {
      current_line = current_line + 1;
      plVar3 = (long *)find_line(current_line);
      if (plVar3 == (long *)0x0) {
        if (*(char *)((long)param_1 + 0x1c) != '\0') {
          if (cVar1 != '\x01') {
            dump_rest_of_file();
            close_output_file();
          }
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        regexp_error(param_1,param_2,cVar1);
      }
      local_30 = *plVar3;
      if (*(char *)(plVar3[1] + local_30 + -1) == '\n') {
        local_30 = local_30 + -1;
      }
      iVar2 = re_search((re_pattern_buffer *)(param_1 + 4),(char *)plVar3[1],(int)local_30,0,
                        (int)local_30,(re_registers *)0x0);
      if (iVar2 == -2) {
        uVar4 = gettext("error in regular expression search");
        error(0,0,uVar4);
                    /* WARNING: Subroutine does not return */
        cleanup_fatal();
      }
    } while (iVar2 == -1);
  }
  else {
    while( true ) {
      current_line = current_line + 1;
      plVar3 = (long *)find_line(current_line);
      if (plVar3 == (long *)0x0) {
        if (*(char *)((long)param_1 + 0x1c) != '\0') {
          if (cVar1 != '\x01') {
            dump_rest_of_file();
            close_output_file();
          }
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        regexp_error(param_1,param_2,cVar1);
      }
      local_30 = *plVar3;
      if (*(char *)(plVar3[1] + local_30 + -1) == '\n') {
        local_30 = local_30 + -1;
      }
      iVar2 = re_search((re_pattern_buffer *)(param_1 + 4),(char *)plVar3[1],(int)local_30,0,
                        (int)local_30,(re_registers *)0x0);
      if (iVar2 == -2) {
        uVar4 = gettext("error in regular expression search");
        error(0,0,uVar4);
                    /* WARNING: Subroutine does not return */
        cleanup_fatal();
      }
      if (iVar2 != -1) break;
      uVar4 = remove_line();
      if (cVar1 != '\x01') {
        save_line_to_file(uVar4);
      }
    }
  }
  lVar5 = current_line + *param_1;
  write_to_file(lVar5,cVar1,*(undefined4 *)(param_1 + 3));
  if (cVar1 != '\x01') {
    close_output_file();
  }
  if (0 < *param_1) {
    current_line = lVar5;
  }
  if (suppress_matched != '\0') {
    remove_line();
  }
  return;
}

// Function: split_file
void split_file(void) {
  long local_18;
  long local_10;
  
  for (local_18 = 0; local_18 < control_used; local_18 = local_18 + 1) {
    if (*(char *)(local_18 * 0x60 + controls + 0x1e) == '\0') {
      for (local_10 = 0;
          (*(char *)(local_18 * 0x60 + controls + 0x1c) != '\0' ||
          (local_10 <= *(long *)(local_18 * 0x60 + controls + 0x10))); local_10 = local_10 + 1) {
        process_line_count(controls + local_18 * 0x60,local_10);
      }
    }
    else {
      for (local_10 = 0;
          (*(char *)(local_18 * 0x60 + controls + 0x1c) != '\0' ||
          (local_10 <= *(long *)(local_18 * 0x60 + controls + 0x10))); local_10 = local_10 + 1) {
        process_regexp(controls + local_18 * 0x60,local_10);
      }
    }
  }
  create_output_file();
  dump_rest_of_file();
  close_output_file();
  return;
}

// Function: make_filename
char * make_filename(uint param_1) {
  uint uVar1;
  char *pcVar2;
  char *__format;
  size_t sVar3;
  
  strcpy(filename_space,prefix);
  __format = suffix;
  pcVar2 = filename_space;
  uVar1 = digits;
  if (suffix == (char *)0x0) {
    sVar3 = strlen(prefix);
    sprintf(pcVar2 + sVar3,"%0*d",(ulong)uVar1,(ulong)param_1);
  }
  else {
    sVar3 = strlen(prefix);
    sprintf(pcVar2 + sVar3,__format,(ulong)param_1);
  }
  return filename_space;
}

// Function: create_output_file
void create_output_file(void) {
  int iVar1;
  int *piVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  bool bVar4;
  int local_b0;
  sigset_t local_98;
  long local_10;
  
  iVar1 = files_created;
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  output_filename = make_filename(files_created);
  if (iVar1 == 0x7fffffff) {
    bVar4 = false;
    local_b0 = 0x4b;
  }
  else {
    sigprocmask(0,(sigset_t *)&caught_signals,&local_98);
    output_stream = fopen_safer(output_filename,&DAT_001035c0);
    bVar4 = output_stream != 0;
    piVar2 = __errno_location();
    local_b0 = *piVar2;
    files_created = iVar1 + (uint)bVar4;
    sigprocmask(2,&local_98,(sigset_t *)0x0);
  }
  if (!bVar4) {
    uVar3 = quotearg_n_style_colon(0,3,output_filename);
    error(0,local_b0,&DAT_00103500,uVar3);
                    /* WARNING: Subroutine does not return */
    cleanup_fatal();
  }
  bytes_written = 0;
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: delete_all_files
void delete_all_files(char param_1) {
  int iVar1;
  char *__name;
  int *piVar2;
  undefined8 uVar3;
  int local_34;
  
  if (remove_files == '\x01') {
    local_34 = files_created;
    while (local_34 = local_34 + -1, -1 < local_34) {
      __name = (char *)make_filename(local_34);
      iVar1 = unlink(__name);
      if (iVar1 != 0) {
        piVar2 = __errno_location();
        if ((*piVar2 != 2) && (param_1 != '\x01')) {
          uVar3 = quotearg_n_style_colon(0,3,__name);
          piVar2 = __errno_location();
          error(0,*piVar2,&DAT_00103500,uVar3);
        }
      }
    }
    files_created = 0;
  }
  return;
}

// Function: close_output_file
void close_output_file(void) {
  int iVar1;
  int iVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  int *piVar5;
  long in_FS_OFFSET;
  sigset_t local_a8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (output_stream != (FILE *)0x0) {
    iVar1 = ferror_unlocked(output_stream);
    if (iVar1 != 0) {
      uVar3 = quotearg_style(4,output_filename);
      uVar4 = gettext("write error for %s");
      error(0,0,uVar4,uVar3);
      output_stream = (FILE *)0x0;
                    /* WARNING: Subroutine does not return */
      cleanup_fatal();
    }
    iVar1 = rpl_fclose(output_stream);
    if (iVar1 != 0) {
      uVar3 = quotearg_n_style_colon(0,3,output_filename);
      piVar5 = __errno_location();
      error(0,*piVar5,&DAT_00103500,uVar3);
      output_stream = (FILE *)0x0;
                    /* WARNING: Subroutine does not return */
      cleanup_fatal();
    }
    if ((bytes_written == 0) && (elide_empty_files != '\0')) {
      sigprocmask(0,(sigset_t *)&caught_signals,&local_a8);
      iVar2 = unlink(output_filename);
      piVar5 = __errno_location();
      iVar1 = *piVar5;
      files_created = files_created + -1;
      sigprocmask(2,&local_a8,(sigset_t *)0x0);
      if ((iVar2 != 0) && (iVar1 != 2)) {
        uVar3 = quotearg_n_style_colon(0,3,output_filename);
        error(0,iVar1,&DAT_00103500,uVar3);
      }
    }
    else if (suppress_count != '\x01') {
      fprintf(_stdout,"%jd\n",bytes_written);
    }
    output_stream = (FILE *)0x0;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: save_line_to_file
void save_line_to_file(size_t *param_1) {
  size_t sVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  
  sVar1 = fwrite_unlocked((void *)param_1[1],1,*param_1,output_stream);
  if (sVar1 != *param_1) {
    uVar2 = quotearg_style(4,output_filename);
    uVar3 = gettext("write error for %s");
    piVar4 = __errno_location();
    error(0,*piVar4,uVar3,uVar2);
    output_stream = (FILE *)0x0;
                    /* WARNING: Subroutine does not return */
    cleanup_fatal();
  }
  bytes_written = bytes_written + *param_1;
  return;
}

// Function: new_control_record
undefined8 * new_control_record(void) {
  undefined8 *puVar1;
  
  if (control_used == control_allocated_2) {
    controls = xpalloc(controls,&control_allocated_2,1,0xffffffffffffffff,0x60);
  }
  puVar1 = (undefined8 *)(control_used * 0x60 + controls);
  control_used = control_used + 1;
  *(undefined *)((long)puVar1 + 0x1e) = 0;
  puVar1[2] = 0;
  *(undefined *)((long)puVar1 + 0x1c) = 0;
  puVar1[1] = 0;
  *puVar1 = 0;
  return puVar1;
}

// Function: check_for_offset
void check_for_offset(undefined8 param_1,undefined8 param_2,undefined8 param_3) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  
  iVar1 = xstrtoimax(param_3,0,10,param_1,&DAT_00103454);
  if (iVar1 != 0) {
    uVar2 = quote(param_2);
    uVar3 = gettext("%s: integer expected after delimiter");
    error(1,0,uVar3,uVar2);
  }
  return;
}

// Function: parse_repeat_count
void parse_repeat_count(int param_1,long param_2,char *param_3) {
  int iVar1;
  size_t sVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  long local_40;
  char *local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  sVar2 = strlen(param_3);
  local_38 = param_3 + (sVar2 - 1);
  if (*local_38 != '}

// Function: extract_regexp
long extract_regexp(undefined4 param_1,undefined param_2,char *param_3) {
  char cVar1;
  char *pcVar2;
  undefined8 uVar3;
  long lVar4;
  char *pcVar5;
  undefined8 uVar6;
  
  cVar1 = *param_3;
  pcVar2 = strrchr(param_3 + 1,(int)cVar1);
  if (pcVar2 == (char *)0x0) {
    uVar3 = gettext("%s: closing delimiter \'%c\' missing");
    error(1,0,uVar3,param_3,(int)cVar1);
  }
  lVar4 = new_control_record();
  *(undefined4 *)(lVar4 + 0x18) = param_1;
  *(undefined *)(lVar4 + 0x1d) = param_2;
  *(undefined *)(lVar4 + 0x1e) = 1;
  *(undefined8 *)(lVar4 + 0x20) = 0;
  *(undefined8 *)(lVar4 + 0x28) = 0;
  uVar3 = xmalloc(0x100);
  *(undefined8 *)(lVar4 + 0x40) = uVar3;
  *(undefined8 *)(lVar4 + 0x48) = 0;
  _re_syntax_options = 0x2c6;
  pcVar5 = re_compile_pattern(param_3 + 1,(size_t)(pcVar2 + (-1 - (long)param_3)),
                              (re_pattern_buffer *)(lVar4 + 0x20));
  if (pcVar5 != (char *)0x0) {
    uVar3 = quote(param_3);
    uVar6 = gettext("%s: invalid regular expression: %s");
    error(0,0,uVar6,uVar3,pcVar5);
                    /* WARNING: Subroutine does not return */
    cleanup_fatal();
  }
  if (pcVar2[1] != '\0') {
    check_for_offset(lVar4,param_3,pcVar2 + 1);
  }
  return lVar4;
}

// Function: parse_patterns
void parse_patterns(int param_1,int param_2,long param_3) {
  ulong uVar1;
  int iVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  int local_74;
  ulong local_50;
  long local_48;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  for (local_74 = param_2; local_74 < param_1; local_74 = local_74 + 1) {
    if ((**(char **)(param_3 + (long)local_74 * 8) == '/') ||
       (**(char **)(param_3 + (long)local_74 * 8) == '%')) {
      local_48 = extract_regexp(local_74,**(char **)(param_3 + (long)local_74 * 8) == '%',
                                *(undefined8 *)(param_3 + (long)local_74 * 8));
    }
    else {
      local_48 = new_control_record();
      *(int *)(local_48 + 0x18) = local_74;
      iVar2 = xstrtoumax(*(undefined8 *)(param_3 + (long)local_74 * 8),0,10,&local_50,&DAT_00103454)
      ;
      if ((iVar2 != 0) || ((long)local_50 < 0)) {
        uVar3 = quote(*(undefined8 *)(param_3 + (long)local_74 * 8));
        local_40 = gettext("%s: invalid pattern");
        error(1,0,local_40,uVar3);
      }
      if (local_50 == 0) {
        uVar3 = *(undefined8 *)(param_3 + (long)local_74 * 8);
        local_38 = gettext("%s: line number must be greater than zero");
        error(1,0,local_38,uVar3);
      }
      uVar1 = last_val_1;
      if (local_50 < last_val_1) {
        uVar3 = quote(*(undefined8 *)(param_3 + (long)local_74 * 8));
        local_30 = gettext("line number %s is smaller than preceding line number, %jd");
        error(1,0,local_30,uVar3,uVar1);
      }
      if (local_50 == last_val_1) {
        uVar3 = quote(*(undefined8 *)(param_3 + (long)local_74 * 8));
        local_28 = gettext("warning: line number %s is the same as preceding line number");
        error(0,0,local_28,uVar3);
      }
      last_val_1 = local_50;
      *(ulong *)(local_48 + 8) = local_50;
    }
    if ((local_74 + 1 < param_1) && (**(char **)(param_3 + ((long)local_74 + 1) * 8) == '{')) {
      local_74 = local_74 + 1;
      parse_repeat_count(local_74,local_48,*(undefined8 *)(param_3 + (long)local_74 * 8));
    }
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: get_format_flags
long get_format_flags(long param_1,uint *param_2)

{
  char cVar1;
  uint local_14;
  long local_10;
  
  local_14 = 0;
  local_10 = 0;
  do {
    cVar1 = *(char *)(param_1 + local_10);
    if (cVar1 != '0') {
      if ('0' < cVar1) goto LAB_00102551;
      if (cVar1 != '-') {
        if ('-' < cVar1) {
LAB_00102551:
          *param_2 = local_14;
          return local_10;
        }
        if (cVar1 == '#') {
          local_14 = local_14 | 2;
        }
        else {
          if (cVar1 != '\'') goto LAB_00102551;
          local_14 = local_14 | 1;
        }
      }
    }
    local_10 = local_10 + 1;
  } while( true );
}

// Function: check_format_conv_type
void check_format_conv_type(byte *param_1,uint param_2)

{
  byte bVar1;
  ushort **ppuVar2;
  undefined8 uVar3;
  ulong uVar4;
  undefined8 uVar5;
  uint local_5c;
  
  bVar1 = *param_1;
  local_5c = 1;
  if (bVar1 == 0) {
    uVar5 = gettext("missing conversion specifier in suffix");
    error(1,0,uVar5);
  }
  else if ((bVar1 < 0x79) && (0x57 < bVar1)) {
    uVar4 = 1L << (bVar1 + 0xa8 & 0x3f);
    if ((uVar4 & 0x100800001) != 0) {
      local_5c = 2;
      goto LAB_001026f0;
    }
    if ((uVar4 & 0x21000) != 0) goto LAB_001026f0;
    if ((uVar4 & 0x20000000) != 0) {
      *param_1 = 100;
      goto LAB_001026f0;
    }
  }
  ppuVar2 = __ctype_b_loc();
  if (((*ppuVar2)[bVar1] & 0x4000) != 0) {
    uVar5 = gettext("invalid conversion specifier in suffix: %c");
    error(1,0,uVar5,bVar1);
  }
  uVar5 = gettext("invalid conversion specifier in suffix: \\%.3o");
  error(1,0,uVar5,bVar1);
LAB_001026f0:
  if ((~local_5c & param_2) != 0) {
    if ((~local_5c & param_2 & 2) == 0) {
      uVar5 = 0x27;
    }
    else {
      uVar5 = 0x23;
    }
    uVar3 = gettext("invalid flags in conversion specification: %%%c%c");
    error(1,0,uVar3,uVar5,bVar1);
  }
  return;
}

// Function: max_out
long max_out(char *param_1)

{
  bool bVar1;
  char cVar2;
  long lVar3;
  long in_FS_OFFSET;
  undefined4 local_40;
  int local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  char *local_28;
  undefined8 local_20;
  undefined8 local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  bVar1 = false;
  for (local_28 = param_1; *local_28 != '\0'; local_28 = local_28 + 1) {
    if ((*local_28 == '%') && (local_28 = local_28 + 1, *local_28 != '%')) {
      if (bVar1) {
        local_20 = gettext("too many %% conversion specifications in suffix");
        local_38 = 1;
        local_34 = 0;
        error(1,0,local_20);
      }
      bVar1 = true;
      lVar3 = get_format_flags(local_28,&local_40);
      local_28 = local_28 + lVar3;
      while (cVar2 = c_isdigit((int)*local_28), cVar2 != '\0') {
        local_28 = local_28 + 1;
      }
      if (*local_28 == '.') {
        do {
          local_28 = local_28 + 1;
          cVar2 = c_isdigit((int)*local_28);
        } while (cVar2 != '\0');
      }
      check_format_conv_type(local_28,local_40);
    }
  }
  if (!bVar1) {
    local_18 = gettext("missing %% conversion specification in suffix");
    local_30 = 1;
    local_2c = 0;
    error(1,0,local_18);
  }
  local_3c = snprintf((char *)0x0,0,param_1,0x7fffffff);
  if (local_3c < 0) {
    xalloc_die();
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return (long)local_3c;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: main
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 main(int param_1,undefined8 *param_2)

{
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  size_t sVar4;
  long lVar5;
  int *piVar6;
  long in_FS_OFFSET;
  int local_108;
  sigaction local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  controls = 0;
  control_used = 0;
  suppress_count = 0;
  remove_files = 1;
  suppress_matched = 0;
  prefix = "xx";
  global_argv = param_2;
  do {
    while( true ) {
      iVar1 = getopt_long(param_1,param_2,"f:b:kn:sqz",longopts,0);
      if (iVar1 == -1) {
        if (param_1 - _optind < 2) {
          if (_optind < param_1) {
            uVar2 = quote(param_2[(long)param_1 + -1]);
            uVar3 = gettext("missing operand after %s");
            error(0,0,uVar3,uVar2);
          }
          else {
            uVar2 = gettext("missing operand");
            error(0,0,uVar2);
          }
          usage(1);
        }
        sVar4 = strlen(prefix);
        if (suffix == (char *)0x0) {
          if (digits < 0xb) {
            lVar5 = 0xb;
          }
          else {
            lVar5 = (long)(int)digits;
          }
        }
        else {
          lVar5 = max_out(suffix);
        }
        if (SCARRY8(sVar4,lVar5 + 1)) {
          xalloc_die();
        }
        filename_space = ximalloc(sVar4 + lVar5 + 1);
        lVar5 = (long)_optind;
        _optind = _optind + 1;
        set_input_file(param_2[lVar5]);
        parse_patterns(param_1,_optind,param_2);
        sigemptyset((sigset_t *)&caught_signals);
        for (local_108 = 0; local_108 < 0xb; local_108 = local_108 + 1) {
          sigaction(*(int *)(sig_0 + (long)local_108 * 4),(sigaction *)0x0,&local_b8);
          if (local_b8.__sigaction_handler.sa_handler != (__sighandler_t)0x1) {
            sigaddset((sigset_t *)&caught_signals,*(int *)(sig_0 + (long)local_108 * 4));
          }
        }
        local_b8.__sigaction_handler.sa_handler = interrupt_handler;
        local_b8.sa_mask.__val[0] = caught_signals;
        local_b8.sa_mask.__val[1] = DAT_00103268;
        local_b8.sa_mask.__val[2] = DAT_00103270;
        local_b8.sa_mask.__val[3] = DAT_00103278;
        local_b8.sa_mask.__val[4] = DAT_00103280;
        local_b8.sa_mask.__val[5] = DAT_00103288;
        local_b8.sa_mask.__val[6] = DAT_00103290;
        local_b8.sa_mask.__val[7] = DAT_00103298;
        local_b8.sa_mask.__val[8] = DAT_001032a0;
        local_b8.sa_mask.__val[9] = DAT_001032a8;
        local_b8.sa_mask.__val[10] = DAT_001032b0;
        local_b8.sa_mask.__val[11] = DAT_001032b8;
        local_b8.sa_mask.__val[12] = DAT_001032c0;
        local_b8.sa_mask.__val[13] = DAT_001032c8;
        local_b8.sa_mask.__val[14] = DAT_001032d0;
        local_b8.sa_mask.__val[15] = DAT_001032d8;
        local_b8.sa_flags = 0;
        for (local_108 = 0; local_108 < 0xb; local_108 = local_108 + 1) {
          iVar1 = sigismember((sigset_t *)&caught_signals,*(int *)(sig_0 + (long)local_108 * 4));
          if (iVar1 != 0) {
            sigaction(*(int *)(sig_0 + (long)local_108 * 4),&local_b8,(sigaction *)0x0);
          }
        }
        split_file();
        iVar1 = close(0);
        if (iVar1 == 0) {
          if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
            return 0;
          }
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        uVar2 = gettext("read error");
        piVar6 = __errno_location();
        error(0,*piVar6,uVar2);
                    /* WARNING: Subroutine does not return */
        cleanup_fatal();
      }
      if (iVar1 < 0x81) break;
switchD_00102a4f_caseD_63:
      usage(1);
    }
    if (iVar1 < 0x62) {
      if (iVar1 == -0x83) {
LAB_00102afb:
        uVar2 = proper_name_lite("David MacKenzie","David MacKenzie");
        uVar3 = proper_name_lite("Stuart Kemp","Stuart Kemp");
        version_etc(_stdout,"csplit","GNU coreutils",_Version,uVar3,uVar2,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar1 == -0x82) {
        usage(0);
        goto LAB_00102afb;
      }
      goto switchD_00102a4f_caseD_63;
    }
    switch(iVar1) {
    case 0x62:
      suffix = _optarg;
      break;
    default:
      goto switchD_00102a4f_caseD_63;
    case 0x66:
      prefix = _optarg;
      break;
    case 0x6b:
      remove_files = 0;
      break;
    case 0x6e:
      uVar2 = gettext("invalid number");
      digits = xdectoimax(_optarg,0,0x7fffffff,&DAT_00103454,uVar2,0);
      break;
    case 0x71:
    case 0x73:
      suppress_count = 1;
      break;
    case 0x7a:
      elide_empty_files = 1;
      break;
    case 0x80:
      suppress_matched = 1;
    }
  } while( true );
}

// Function: usage
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void usage(int param_1)

{
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... FILE PATTERN...\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Output pieces of FILE separated by PATTERN(s) to files \'xx00\', \'xx01\', ...,\nand output byte counts of each piece to standard output.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("\nRead standard input if FILE is -\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -b, --suffix-format=FORMAT  use sprintf FORMAT instead of %02d\n  -f, --prefix=PREFIX        use PREFIX instead of \'xx\'\n  -k, --keep-files           do not remove output files on errors\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --suppress-matched     suppress the lines matching PATTERN\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -n, --digits=DIGITS        use specified number of digits instead of 2\n  -s, --quiet, --silent      do not print counts of output file sizes\n  -z, --elide-empty-files    suppress empty output files\n"
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
                            "\nEach PATTERN may be:\n  INTEGER            copy up to but not including specified line number\n  /REGEXP/[OFFSET]   copy up to but not including a matching line\n  %REGEXP%[OFFSET]   skip to, but not including a matching line\n  {INTEGER}          repeat the previous pattern specified number of times\n  {*}                repeat the previous pattern as many times as possible\n\nA line OFFSET is an integer optionally preceded by \'+\' or \'-\'\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info("csplit");
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: <EXTERNAL>::gettext
/* WARNING: Control flow encountered bad instruction data */

void gettext(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::fputs_unlocked
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int fputs_unlocked(char *__s,FILE *__stream)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::streq
/* WARNING: Control flow encountered bad instruction data */

void streq(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::emit_bug_reporting_address
/* WARNING: Control flow encountered bad instruction data */

void emit_bug_reporting_address(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::setlocale
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * setlocale(int __category,char *__locale)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::strncmp
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int strncmp(char *__s1,char *__s2,size_t __n)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::printf
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int printf(char *__format,...)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::__stack_chk_fail
/* WARNING: Control flow encountered bad instruction data */

void __stack_chk_fail(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::sigprocmask
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int sigprocmask(int __how,sigset_t *__set,sigset_t *__oset)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::exit
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void exit(int __status)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::error
/* WARNING: Control flow encountered bad instruction data */

void error(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::signal
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

__sighandler_t signal(int __sig,__sighandler_t __handler)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::raise
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int raise(int __sig)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::free
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void free(void *__ptr)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::safe_read
/* WARNING: Control flow encountered bad instruction data */

void safe_read(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::__errno_location
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int * __errno_location(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::xmalloc
/* WARNING: Control flow encountered bad instruction data */

void xmalloc(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::rawmemchr
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void * rawmemchr(void *__s,int __c)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::ximemdup
/* WARNING: Control flow encountered bad instruction data */

void ximemdup(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::xpalloc
/* WARNING: Control flow encountered bad instruction data */

void xpalloc(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::mempcpy
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void * mempcpy(void *__dest,void *__src,size_t __n)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::fd_reopen
/* WARNING: Control flow encountered bad instruction data */

void fd_reopen(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::quotearg_style
/* WARNING: Control flow encountered bad instruction data */

void quotearg_style(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::quote
/* WARNING: Control flow encountered bad instruction data */

void quote(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::imaxtostr
/* WARNING: Control flow encountered bad instruction data */

void imaxtostr(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::fprintf
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int fprintf(FILE *__stream,char *__format,...)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::fputc
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int fputc(int __c,FILE *__stream)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::re_search
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int re_search(re_pattern_buffer *__buffer,char *__string,int __length,int __start,int __range,
             re_registers *__regs)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::strcpy
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * strcpy(char *__dest,char *__src)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::strlen
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

size_t strlen(char *__s)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::sprintf
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int sprintf(char *__s,char *__format,...)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::fopen_safer
/* WARNING: Control flow encountered bad instruction data */

void fopen_safer(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::quotearg_n_style_colon
/* WARNING: Control flow encountered bad instruction data */

void quotearg_n_style_colon(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::unlink
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int unlink(char *__name)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::ferror_unlocked
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int ferror_unlocked(FILE *__stream)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::rpl_fclose
/* WARNING: Control flow encountered bad instruction data */

void rpl_fclose(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::fwrite_unlocked
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

size_t fwrite_unlocked(void *__ptr,size_t __size,size_t __n,FILE *__stream)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::xstrtoimax
/* WARNING: Control flow encountered bad instruction data */

void xstrtoimax(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::xstrtoumax
/* WARNING: Control flow encountered bad instruction data */

void xstrtoumax(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::strrchr
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * strrchr(char *__s,int __c)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::re_compile_pattern
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * re_compile_pattern(char *__pattern,size_t __length,re_pattern_buffer *__buffer)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::__ctype_b_loc
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

ushort ** __ctype_b_loc(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::c_isdigit
/* WARNING: Control flow encountered bad instruction data */

void c_isdigit(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::snprintf
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int snprintf(char *__s,size_t __maxlen,char *__format,...)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::set_program_name
/* WARNING: Control flow encountered bad instruction data */

void set_program_name(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::bindtextdomain
/* WARNING: Control flow encountered bad instruction data */

void bindtextdomain(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::textdomain
/* WARNING: Control flow encountered bad instruction data */

void textdomain(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::atexit
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int atexit(__func *__func)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::xdectoimax
/* WARNING: Control flow encountered bad instruction data */

void xdectoimax(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::proper_name_lite
/* WARNING: Control flow encountered bad instruction data */

void proper_name_lite(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::version_etc
/* WARNING: Control flow encountered bad instruction data */

void version_etc(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::getopt_long
/* WARNING: Control flow encountered bad instruction data */

void getopt_long(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::ximalloc
/* WARNING: Control flow encountered bad instruction data */

void ximalloc(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::sigemptyset
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int sigemptyset(sigset_t *__set)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::sigaction
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int sigaction(int __sig,sigaction *__act,sigaction *__oact)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::sigaddset
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int sigaddset(sigset_t *__set,int __signo)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::sigismember
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int sigismember(sigset_t *__set,int __signo)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: <EXTERNAL>::close
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int close(int __fd)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

// Function: get_format_flags
long get_format_flags(long param_1,uint *param_2) {
  char cVar1;
  uint local_14;
  long local_10;
  
  local_14 = 0;
  local_10 = 0;
  do {
    cVar1 = *(char *)(param_1 + local_10);
    if (cVar1 != '0') {
      if ('0' < cVar1) goto LAB_00102551;
      if (cVar1 != '-') {
        if ('-' < cVar1) {
LAB_00102551:
          *param_2 = local_14;
          return local_10;
        }
        if (cVar1 == '#') {
          local_14 = local_14 | 2;
        }
        else {
          if (cVar1 != '\'') goto LAB_00102551;
          local_14 = local_14 | 1;
        }
      }
    }
    local_10 = local_10 + 1;
  } while( true );
}

// Function: check_format_conv_type
void check_format_conv_type(byte *param_1,uint param_2) {
  byte bVar1;
  ushort **ppuVar2;
  undefined8 uVar3;
  ulong uVar4;
  undefined8 uVar5;
  uint local_5c;
  
  bVar1 = *param_1;
  local_5c = 1;
  if (bVar1 == 0) {
    uVar5 = gettext("missing conversion specifier in suffix");
    error(1,0,uVar5);
  }
  else if ((bVar1 < 0x79) && (0x57 < bVar1)) {
    uVar4 = 1L << (bVar1 + 0xa8 & 0x3f);
    if ((uVar4 & 0x100800001) != 0) {
      local_5c = 2;
      goto LAB_001026f0;
    }
    if ((uVar4 & 0x21000) != 0) goto LAB_001026f0;
    if ((uVar4 & 0x20000000) != 0) {
      *param_1 = 100;
      goto LAB_001026f0;
    }
  }
  ppuVar2 = __ctype_b_loc();
  if (((*ppuVar2)[bVar1] & 0x4000) != 0) {
    uVar5 = gettext("invalid conversion specifier in suffix: %c");
    error(1,0,uVar5,bVar1);
  }
  uVar5 = gettext("invalid conversion specifier in suffix: \\%.3o");
  error(1,0,uVar5,bVar1);
LAB_001026f0:
  if ((~local_5c & param_2) != 0) {
    if ((~local_5c & param_2 & 2) == 0) {
      uVar5 = 0x27;
    }
    else {
      uVar5 = 0x23;
    }
    uVar3 = gettext("invalid flags in conversion specification: %%%c%c");
    error(1,0,uVar3,uVar5,bVar1);
  }
  return;
}

// Function: max_out
long max_out(char *param_1) {
  bool bVar1;
  char cVar2;
  long lVar3;
  long in_FS_OFFSET;
  undefined4 local_40;
  int local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  char *local_28;
  undefined8 local_20;
  undefined8 local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  bVar1 = false;
  for (local_28 = param_1; *local_28 != '\0'; local_28 = local_28 + 1) {
    if ((*local_28 == '%') && (local_28 = local_28 + 1, *local_28 != '%')) {
      if (bVar1) {
        local_20 = gettext("too many %% conversion specifications in suffix");
        local_38 = 1;
        local_34 = 0;
        error(1,0,local_20);
      }
      bVar1 = true;
      lVar3 = get_format_flags(local_28,&local_40);
      local_28 = local_28 + lVar3;
      while (cVar2 = c_isdigit((int)*local_28), cVar2 != '\0') {
        local_28 = local_28 + 1;
      }
      if (*local_28 == '.') {
        do {
          local_28 = local_28 + 1;
          cVar2 = c_isdigit((int)*local_28);
        } while (cVar2 != '\0');
      }
      check_format_conv_type(local_28,local_40);
    }
  }
  if (!bVar1) {
    local_18 = gettext("missing %% conversion specification in suffix");
    local_30 = 1;
    local_2c = 0;
    error(1,0,local_18);
  }
  local_3c = snprintf((char *)0x0,0,param_1,0x7fffffff);
  if (local_3c < 0) {
    xalloc_die();
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return (long)local_3c;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: main
undefined8 main(int param_1,undefined8 *param_2) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  size_t sVar4;
  long lVar5;
  int *piVar6;
  long in_FS_OFFSET;
  int local_108;
  sigaction local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  controls = 0;
  control_used = 0;
  suppress_count = 0;
  remove_files = 1;
  suppress_matched = 0;
  prefix = "xx";
  global_argv = param_2;
  do {
    while( true ) {
      iVar1 = getopt_long(param_1,param_2,"f:b:kn:sqz",longopts,0);
      if (iVar1 == -1) {
        if (param_1 - _optind < 2) {
          if (_optind < param_1) {
            uVar2 = quote(param_2[(long)param_1 + -1]);
            uVar3 = gettext("missing operand after %s");
            error(0,0,uVar3,uVar2);
          }
          else {
            uVar2 = gettext("missing operand");
            error(0,0,uVar2);
          }
          usage(1);
        }
        sVar4 = strlen(prefix);
        if (suffix == (char *)0x0) {
          if (digits < 0xb) {
            lVar5 = 0xb;
          }
          else {
            lVar5 = (long)(int)digits;
          }
        }
        else {
          lVar5 = max_out(suffix);
        }
        if (SCARRY8(sVar4,lVar5 + 1)) {
          xalloc_die();
        }
        filename_space = ximalloc(sVar4 + lVar5 + 1);
        lVar5 = (long)_optind;
        _optind = _optind + 1;
        set_input_file(param_2[lVar5]);
        parse_patterns(param_1,_optind,param_2);
        sigemptyset((sigset_t *)&caught_signals);
        for (local_108 = 0; local_108 < 0xb; local_108 = local_108 + 1) {
          sigaction(*(int *)(sig_0 + (long)local_108 * 4),(sigaction *)0x0,&local_b8);
          if (local_b8.__sigaction_handler.sa_handler != (__sighandler_t)0x1) {
            sigaddset((sigset_t *)&caught_signals,*(int *)(sig_0 + (long)local_108 * 4));
          }
        }
        local_b8.__sigaction_handler.sa_handler = interrupt_handler;
        local_b8.sa_mask.__val[0] = caught_signals;
        local_b8.sa_mask.__val[1] = DAT_00103268;
        local_b8.sa_mask.__val[2] = DAT_00103270;
        local_b8.sa_mask.__val[3] = DAT_00103278;
        local_b8.sa_mask.__val[4] = DAT_00103280;
        local_b8.sa_mask.__val[5] = DAT_00103288;
        local_b8.sa_mask.__val[6] = DAT_00103290;
        local_b8.sa_mask.__val[7] = DAT_00103298;
        local_b8.sa_mask.__val[8] = DAT_001032a0;
        local_b8.sa_mask.__val[9] = DAT_001032a8;
        local_b8.sa_mask.__val[10] = DAT_001032b0;
        local_b8.sa_mask.__val[11] = DAT_001032b8;
        local_b8.sa_mask.__val[12] = DAT_001032c0;
        local_b8.sa_mask.__val[13] = DAT_001032c8;
        local_b8.sa_mask.__val[14] = DAT_001032d0;
        local_b8.sa_mask.__val[15] = DAT_001032d8;
        local_b8.sa_flags = 0;
        for (local_108 = 0; local_108 < 0xb; local_108 = local_108 + 1) {
          iVar1 = sigismember((sigset_t *)&caught_signals,*(int *)(sig_0 + (long)local_108 * 4));
          if (iVar1 != 0) {
            sigaction(*(int *)(sig_0 + (long)local_108 * 4),&local_b8,(sigaction *)0x0);
          }
        }
        split_file();
        iVar1 = close(0);
        if (iVar1 == 0) {
          if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
            return 0;
          }
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        uVar2 = gettext("read error");
        piVar6 = __errno_location();
        error(0,*piVar6,uVar2);
                    /* WARNING: Subroutine does not return */
        cleanup_fatal();
      }
      if (iVar1 < 0x81) break;
switchD_00102a4f_caseD_63:
      usage(1);
    }
    if (iVar1 < 0x62) {
      if (iVar1 == -0x83) {
LAB_00102afb:
        uVar2 = proper_name_lite("David MacKenzie","David MacKenzie");
        uVar3 = proper_name_lite("Stuart Kemp","Stuart Kemp");
        version_etc(_stdout,"csplit","GNU coreutils",_Version,uVar3,uVar2,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar1 == -0x82) {
        usage(0);
        goto LAB_00102afb;
      }
      goto switchD_00102a4f_caseD_63;
    }
    switch(iVar1) {
    case 0x62:
      suffix = _optarg;
      break;
    default:
      goto switchD_00102a4f_caseD_63;
    case 0x66:
      prefix = _optarg;
      break;
    case 0x6b:
      remove_files = 0;
      break;
    case 0x6e:
      uVar2 = gettext("invalid number");
      digits = xdectoimax(_optarg,0,0x7fffffff,&DAT_00103454,uVar2,0);
      break;
    case 0x71:
    case 0x73:
      suppress_count = 1;
      break;
    case 0x7a:
      elide_empty_files = 1;
      break;
    case 0x80:
      suppress_matched = 1;
    }
  } while( true );
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... FILE PATTERN...\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Output pieces of FILE separated by PATTERN(s) to files \'xx00\', \'xx01\', ...,\nand output byte counts of each piece to standard output.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("\nRead standard input if FILE is -\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -b, --suffix-format=FORMAT  use sprintf FORMAT instead of %02d\n  -f, --prefix=PREFIX        use PREFIX instead of \'xx\'\n  -k, --keep-files           do not remove output files on errors\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --suppress-matched     suppress the lines matching PATTERN\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -n, --digits=DIGITS        use specified number of digits instead of 2\n  -s, --quiet, --silent      do not print counts of output file sizes\n  -z, --elide-empty-files    suppress empty output files\n"
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
                            "\nEach PATTERN may be:\n  INTEGER            copy up to but not including specified line number\n  /REGEXP/[OFFSET]   copy up to but not including a matching line\n  %REGEXP%[OFFSET]   skip to, but not including a matching line\n  {INTEGER}          repeat the previous pattern specified number of times\n  {*}                repeat the previous pattern as many times as possible\n\nA line OFFSET is an integer optionally preceded by \'+\' or \'-\'\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info("csplit");
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

