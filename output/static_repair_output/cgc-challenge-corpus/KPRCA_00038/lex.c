// Function: program_init
void program_init(undefined4 *param_1,undefined4 param_2) {
  memset(param_1,0,8);
  *param_1 = param_2;
  return;
}

// Function: program_parse
undefined4 program_parse(undefined4 *param_1) {
  int iVar1;
  int local_1c;
  int *local_18;
  int local_14;
  int local_10;
  
  local_10 = 0;
  do {
    io_mark(*param_1);
    local_14 = parse_pattern(param_1);
    if ((local_14 == 0) || (iVar1 = parse_statements(param_1,&local_1c), iVar1 == 0)) {
      free_pattern(local_14);
      return 0;
    }
    local_18 = (int *)malloc(0xc);
    if (local_18 == (int *)0x0) {
      return 1;
    }
    *local_18 = local_14;
    local_18[1] = local_1c;
    local_18[2] = 0;
    if (local_10 == 0) {
      param_1[1] = local_18;
      local_10 = param_1[1];
    }
    else {
      *(int **)(local_10 + 8) = local_18;
      local_10 = *(int *)(local_10 + 8);
    }
  } while (local_14 != -3);
  return 1;
}

// Function: parse_pattern
undefined4 parse_pattern(undefined4 *param_1) {
  int iVar1;
  undefined4 local_14;
  undefined4 local_10;
  
  iVar1 = pattern_match(param_1,"BEGIN");
  if (iVar1 == 0) {
    iVar1 = pattern_match(param_1,&DAT_0001b16e);
    if (iVar1 == 0) {
      iVar1 = pattern_match(param_1,&DAT_0001b172);
      if (iVar1 == 0) {
        local_10 = io_tell(*param_1);
        iVar1 = parse_expression(param_1,&local_14,0x7b);
        if (iVar1 == 0) {
          io_seek(*param_1,local_10);
          local_14 = 0;
        }
        else {
          io_ungetc(*param_1);
        }
      }
      else {
        local_14 = 0xffffffff;
      }
    }
    else {
      local_14 = 0xfffffffd;
    }
  }
  else {
    local_14 = 0xfffffffe;
  }
  return local_14;
}

// Function: pattern_match
undefined4 pattern_match(undefined4 *param_1,int param_2) {
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int local_10;
  
  uVar1 = io_tell(*param_1);
  do {
    iVar3 = io_getc(*param_1);
    if (iVar3 < 0) break;
    iVar2 = isspace(iVar3);
  } while (iVar2 != 0);
  if (-1 < iVar3) {
    io_ungetc(*param_1);
    for (local_10 = 0; *(char *)(local_10 + param_2) != '\0'; local_10 = local_10 + 1) {
      iVar3 = io_getc(*param_1);
      if ((iVar3 < 0) || (iVar3 != *(char *)(local_10 + param_2))) goto LAB_000149fd;
    }
    skip_whitespace(param_1);
    iVar3 = io_peek(*param_1);
    if (iVar3 == 0x7b) {
      return 1;
    }
  }
LAB_000149fd:
  io_seek(*param_1,uVar1);
  return 0;
}

// Function: free_pattern
undefined ** free_pattern(int param_1) {
  undefined **ppuVar1;
  
  ppuVar1 = &_GLOBAL_OFFSET_TABLE_;
  if ((((param_1 != 0) && (param_1 != -1)) && (param_1 != -2)) && (param_1 != -3)) {
    ppuVar1 = (undefined **)free_expression(param_1);
  }
  return ppuVar1;
}

// Function: free_expression
undefined ** free_expression(void) {
  return &_GLOBAL_OFFSET_TABLE_;
}

// Function: parse_statements
undefined4 parse_statements(undefined4 *param_1,undefined4 *param_2) {
  int iVar1;
  undefined4 local_30;
  undefined4 *local_2c;
  int local_28;
  int local_24;
  undefined4 local_20;
  int local_1c;
  undefined4 *local_18;
  undefined4 *local_14;
  char *local_10;
  
  local_20 = io_tell(*param_1);
  local_10 = (char *)0x0;
  local_14 = (undefined4 *)0x0;
  skip_whitespace(param_1);
  iVar1 = io_getc(*param_1);
  if (iVar1 == 0x7b) {
    local_18 = (undefined4 *)0x0;
    while( true ) {
      while( true ) {
        skip_whitespace(param_1);
        local_28 = io_peek(*param_1);
        if (local_28 != 0x23) break;
        skip_line(param_1);
      }
      if (local_28 == 0x7d) {
        io_getc(*param_1);
        *param_2 = local_14;
        return 1;
      }
      iVar1 = parse_statements(param_1,&local_2c);
      if (iVar1 == 0) break;
      if (local_14 == (undefined4 *)0x0) {
        local_14 = local_2c;
        local_18 = local_2c;
      }
      else {
        local_18[1] = local_2c;
        local_18 = (undefined4 *)local_18[1];
      }
    }
    goto LAB_00015280;
  }
  local_24 = io_tell(*param_1);
  local_24 = local_24 + -1;
  io_seek(*param_1,local_24);
  local_10 = (char *)parse_var(param_1);
  if (local_10 == (char *)0x0) goto LAB_00015280;
  skip_whitespace(param_1);
  local_14 = (undefined4 *)calloc(1,0x18);
  iVar1 = strcmp(local_10,"if");
  if (iVar1 == 0) {
    *local_14 = 1;
    iVar1 = io_getc(*param_1);
    if ((iVar1 != 0x28) || (iVar1 = parse_expression(param_1,local_14 + 2,0x29), iVar1 == 0))
    goto LAB_00015280;
    iVar1 = parse_statements(param_1,local_14 + 3);
  }
  else {
    iVar1 = strcmp(local_10,"while");
    if (iVar1 != 0) {
      iVar1 = strcmp(local_10,"do");
      if (iVar1 == 0) {
        *local_14 = 2;
        local_14[2] = 1;
        iVar1 = parse_statements(param_1,local_14 + 4);
        if (iVar1 == 0) goto LAB_00015280;
        free(local_10);
        skip_whitespace(param_1);
        local_10 = (char *)parse_var(param_1);
        if ((local_10 == (char *)0x0) || (iVar1 = strcmp(local_10,"while"), iVar1 != 0))
        goto LAB_00015280;
        skip_whitespace(param_1);
        iVar1 = io_getc(*param_1);
        if ((iVar1 != 0x28) || (iVar1 = parse_expression(param_1,local_14 + 3,0x29), iVar1 == 0))
        goto LAB_00015280;
        skip_whitespace(param_1);
        iVar1 = io_getc(*param_1);
      }
      else {
        iVar1 = strcmp(local_10,"for");
        if (iVar1 == 0) {
          *local_14 = 3;
          iVar1 = io_getc(*param_1);
          if ((((iVar1 != 0x28) || (iVar1 = parse_expression(param_1,local_14 + 2,0x3b), iVar1 == 0)
               ) || (iVar1 = parse_expression(param_1,local_14 + 3,0x3b), iVar1 == 0)) ||
             (iVar1 = parse_expression(param_1,local_14 + 4,0x29), iVar1 == 0)) goto LAB_00015280;
          iVar1 = parse_statements(param_1,local_14 + 5);
          goto joined_r0x00014f9a;
        }
        iVar1 = strcmp(local_10,"continue");
        if (iVar1 == 0) {
          *local_14 = 5;
          skip_whitespace(param_1);
          iVar1 = io_getc(*param_1);
        }
        else {
          iVar1 = strcmp(local_10,"break");
          if (iVar1 == 0) {
            *local_14 = 6;
            skip_whitespace(param_1);
            iVar1 = io_getc(*param_1);
          }
          else {
            iVar1 = strcmp(local_10,"next");
            if (iVar1 == 0) {
              *local_14 = 7;
              skip_whitespace(param_1);
              iVar1 = io_getc(*param_1);
            }
            else {
              iVar1 = strcmp(local_10,"exit");
              if (iVar1 != 0) {
                iVar1 = strcmp(local_10,"printf");
                if ((iVar1 == 0) || (iVar1 = strcmp(local_10,"print"), iVar1 == 0)) {
                  local_1c = 0;
                  *local_14 = 9;
                  iVar1 = strcmp(local_10,"printf");
                  if (iVar1 == 0) {
                    iVar1 = parse_expression(param_1,local_14 + 2,0x101);
                    if (iVar1 == 0) goto LAB_00015280;
                    io_ungetc(*param_1);
                    iVar1 = io_getc(*param_1);
                    if (iVar1 != 0x2c) goto LAB_00015280;
                  }
                  do {
                    iVar1 = parse_expression(param_1,&local_30,0x101);
                    if (iVar1 == 0) goto LAB_00015280;
                    if (local_1c == 0) {
                      local_14[3] = local_30;
                      local_1c = local_14[3];
                    }
                    else {
                      *(undefined4 *)(local_1c + 4) = local_30;
                      local_1c = *(int *)(local_1c + 4);
                    }
                    io_ungetc(*param_1);
                    iVar1 = io_getc(*param_1);
                  } while (iVar1 == 0x2c);
                  goto LAB_0001523d;
                }
                io_seek(*param_1,local_24);
                *local_14 = 10;
                iVar1 = parse_expression(param_1,local_14 + 2,0x100);
                goto joined_r0x00014f9a;
              }
              *local_14 = 8;
              skip_whitespace(param_1);
              iVar1 = io_getc(*param_1);
            }
          }
        }
      }
      if (iVar1 == 0x3b) goto LAB_0001523d;
      goto LAB_00015280;
    }
    *local_14 = 2;
    local_14[2] = 0;
    iVar1 = io_getc(*param_1);
    if ((iVar1 != 0x28) || (iVar1 = parse_expression(param_1,local_14 + 3,0x29), iVar1 == 0))
    goto LAB_00015280;
    iVar1 = parse_statements(param_1,local_14 + 4);
  }
joined_r0x00014f9a:
  if (iVar1 != 0) {
LAB_0001523d:
    *param_2 = local_14;
    return 1;
  }
LAB_00015280:
  free(local_10);
  free_statement(local_14);
  io_seek(*param_1,local_20);
  return 0;
}

// Function: parse_expression
undefined4 parse_expression(undefined4 *param_1,undefined4 *param_2,int param_3) {
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 *local_20;
  undefined4 *local_1c;
  undefined4 local_18;
  int local_14;
  undefined4 *local_10;
  
  local_1c = (undefined4 *)0x0;
  local_10 = (undefined4 *)0x0;
  skip_whitespace(param_1);
  while( true ) {
    local_20 = (undefined4 *)0x0;
    skip_whitespace(param_1);
    local_14 = io_getc(*param_1);
    if (((local_14 == param_3) || ((param_3 == 0x100 && ((local_14 == 10 || (local_14 == 0x3b))))))
       || ((param_3 == 0x101 && (((local_14 == 10 || (local_14 == 0x3b)) || (local_14 == 0x2c))))))
    break;
    switch(local_14) {
    case 0x21:
      local_14 = io_getc(*param_1);
      if (local_14 == 0x3d) {
        local_20 = (undefined4 *)init_expression(0x17);
      }
      else if (local_14 == 0x7e) {
        local_20 = (undefined4 *)init_expression(0x11);
      }
      else {
        io_ungetc(*param_1);
        local_20 = (undefined4 *)init_expression(0x1f);
      }
      break;
    case 0x22:
      io_ungetc(*param_1);
      puVar2 = (undefined4 *)parse_quoted_string(param_1,&local_20);
      goto joined_r0x00015497;
    default:
      io_ungetc(*param_1);
      local_18 = io_tell(*param_1);
      iVar1 = parse_number(param_1,&local_20);
      if (iVar1 == 0) {
        io_seek(*param_1,local_18);
        puVar2 = (undefined4 *)parse_variable(param_1,&local_20);
        goto joined_r0x00015497;
      }
      break;
    case 0x24:
      local_18 = io_tell(*param_1);
      iVar1 = parse_number(param_1,&local_20);
      if (iVar1 == 0) {
        io_seek(*param_1,local_18);
        iVar1 = parse_variable(param_1,&local_20);
        if (iVar1 == 0) goto LAB_000159f1;
        *local_20 = 5;
      }
      else {
        *local_20 = 4;
      }
      break;
    case 0x25:
      local_14 = io_getc(*param_1);
      if (local_14 == 0x3d) {
        local_20 = (undefined4 *)init_expression(0xc);
      }
      else {
        io_ungetc(*param_1);
        local_20 = (undefined4 *)init_expression(0x1d);
      }
      break;
    case 0x26:
      iVar1 = io_getc(*param_1);
      if (iVar1 != 0x26) goto LAB_000159f1;
      local_20 = (undefined4 *)init_expression(0xf);
      break;
    case 0x28:
      puVar2 = (undefined4 *)parse_expression(param_1,&local_20,0x29);
      goto joined_r0x00015497;
    case 0x2a:
      local_14 = io_getc(*param_1);
      if (local_14 == 0x3d) {
        local_20 = (undefined4 *)init_expression(10);
      }
      else {
        io_ungetc(*param_1);
        local_20 = (undefined4 *)init_expression(0x1b);
      }
      break;
    case 0x2b:
      local_14 = io_getc(*param_1);
      if (local_14 == 0x2b) {
        local_20 = (undefined4 *)init_expression(0x26);
      }
      else if (local_14 == 0x3d) {
        local_20 = (undefined4 *)init_expression(8);
      }
      else {
        io_ungetc(*param_1);
        local_20 = (undefined4 *)init_expression(0x19);
      }
      break;
    case 0x2d:
      local_14 = io_getc(*param_1);
      if (local_14 == 0x2d) {
        local_20 = (undefined4 *)init_expression(0x25);
      }
      else if (local_14 == 0x3d) {
        local_20 = (undefined4 *)init_expression(9);
      }
      else {
        io_ungetc(*param_1);
        local_20 = (undefined4 *)init_expression(0x27);
      }
      break;
    case 0x2f:
      if (((local_10 == (undefined4 *)0x0) || (iVar1 = is_unary(*local_10), iVar1 != 0)) ||
         (iVar1 = is_binary(*local_10), iVar1 != 0)) {
        io_ungetc(*param_1);
        puVar2 = (undefined4 *)parse_regexp(param_1,&local_20);
        goto joined_r0x00015497;
      }
      local_14 = io_getc(*param_1);
      if (local_14 == 0x3d) {
        local_20 = (undefined4 *)init_expression(0xb);
      }
      else {
        io_ungetc(*param_1);
        local_20 = (undefined4 *)init_expression(0x1c);
      }
      break;
    case 0x3c:
      iVar1 = io_getc(*param_1);
      if (iVar1 == 0x3d) {
        local_20 = (undefined4 *)init_expression(0x14);
        puVar2 = local_20;
      }
      else {
        io_ungetc(*param_1);
        local_20 = (undefined4 *)init_expression(0x12);
        puVar2 = local_20;
      }
      goto joined_r0x00015497;
    case 0x3d:
      iVar1 = io_getc(*param_1);
      if (iVar1 == 0x3d) {
        local_20 = (undefined4 *)init_expression(0x16);
        if (local_20 == (undefined4 *)0x0) {
          local_20 = (undefined4 *)0x0;
          goto LAB_000159f1;
        }
      }
      else {
        io_ungetc(*param_1);
        local_20 = (undefined4 *)init_expression(7);
        if (local_20 == (undefined4 *)0x0) {
          local_20 = (undefined4 *)0x0;
          goto LAB_000159f1;
        }
      }
      break;
    case 0x3e:
      iVar1 = io_getc(*param_1);
      if (iVar1 == 0x3d) {
        local_20 = (undefined4 *)init_expression(0x15);
        puVar2 = local_20;
      }
      else {
        io_ungetc(*param_1);
        local_20 = (undefined4 *)init_expression(0x13);
        puVar2 = local_20;
      }
      goto joined_r0x00015497;
    case 0x3f:
      local_20 = (undefined4 *)init_expression(0xd);
      if (local_20 == (undefined4 *)0x0) goto LAB_000159f1;
      puVar2 = (undefined4 *)parse_expression(param_1,local_20 + 4,0x3a);
joined_r0x00015497:
      if (puVar2 == (undefined4 *)0x0) goto LAB_000159f1;
      break;
    case 0x5c:
      iVar1 = io_getc(*param_1);
      if (iVar1 != 10) goto LAB_000159f1;
      break;
    case 0x7c:
      iVar1 = io_getc(*param_1);
      if (iVar1 != 0x7c) goto LAB_000159f1;
      local_20 = (undefined4 *)init_expression(0xe);
      break;
    case 0x7e:
      local_20 = (undefined4 *)init_expression(0x10);
    }
    if (local_20 != (undefined4 *)0x0) {
      if (local_10 == (undefined4 *)0x0) {
        local_10 = local_20;
        local_1c = local_20;
        local_20[1] = 0;
        local_20[2] = 0;
      }
      else {
        local_10[1] = local_20;
        local_20[2] = local_10;
        local_20[1] = 0;
        local_10 = local_20;
      }
    }
  }
  iVar1 = treeify_expression(&local_1c,99);
  if (iVar1 == 0) {
LAB_000159f1:
    free_expression(local_1c);
    uVar3 = 0;
  }
  else {
    *param_2 = local_1c;
    uVar3 = 1;
  }
  return uVar3;
}

// Function: parse_variable
undefined4 parse_variable(undefined4 param_1,int *param_2) {
  char *__s;
  size_t sVar1;
  int iVar2;
  undefined4 uVar3;
  
  __s = (char *)parse_var(param_1);
  if ((((__s == (char *)0x0) || (sVar1 = strlen(__s), sVar1 == 0)) ||
      (iVar2 = is_keyword(__s), iVar2 != 0)) || (iVar2 = init_expression(6), iVar2 == 0)) {
    free(__s);
    uVar3 = 0;
  }
  else {
    *(char **)(iVar2 + 0xc) = __s;
    *param_2 = iVar2;
    uVar3 = 1;
  }
  return uVar3;
}

// Function: parse_number
undefined4 parse_number(undefined4 *param_1,void **param_2) {
  char *pcVar1;
  int iVar2;
  undefined4 uStack_60;
  char local_5c [64];
  long local_1c;
  int local_18;
  void *local_14;
  uint local_10;
  
  uStack_60 = 0x15abb;
  local_10 = 0;
  local_14 = (void *)0x0;
  do {
    local_18 = io_getc(*param_1);
    if (local_18 < 0) goto LAB_00015b84;
    pcVar1 = local_5c + local_10;
    local_10 = local_10 + 1;
    *pcVar1 = (char)local_18;
  } while ((local_10 < 0x40) && (iVar2 = isdigit(local_18), iVar2 != 0));
  io_ungetc(*param_1);
  if (1 < (int)local_10) {
    local_5c[local_10 - 1] = '\0';
    local_1c = strtol(local_5c,(char **)0x0,10);
    local_14 = (void *)init_expression(2);
    if (local_14 != (void *)0x0) {
      *(long *)((int)local_14 + 0xc) = local_1c;
      *param_2 = local_14;
      return 1;
    }
  }
LAB_00015b84:
  free(local_14);
  return 0;
}

// Function: parse_regexp
undefined4 parse_regexp(undefined4 *param_1,void **param_2) {
  int iVar1;
  undefined4 uVar2;
  undefined local_20 [12];
  int local_14;
  void *local_10;
  
  local_10 = (void *)0x0;
  strio_init(local_20);
  io_getc(*param_1);
  do {
    local_14 = io_getc(*param_1);
    if (local_14 < 0) {
LAB_00015cae:
      free(local_10);
      strio_free(local_20);
      return 0;
    }
    if (local_14 == 0x2f) {
      local_10 = (void *)init_expression(3);
      if (local_10 != (void *)0x0) {
        uVar2 = strio_dup(local_20);
        *(undefined4 *)((int)local_10 + 0xc) = uVar2;
        if (*(int *)((int)local_10 + 0xc) != 0) {
          strio_free(local_20);
          *param_2 = local_10;
          return 1;
        }
      }
      goto LAB_00015cae;
    }
    if (local_14 == 0x5c) {
      iVar1 = io_getc(*param_1);
      if (iVar1 == 0x2f) {
        strio_append_char(local_20,0x2f);
      }
      else {
        io_ungetc(*param_1);
      }
    }
    else {
      strio_append_char(local_20,(int)(char)local_14);
    }
  } while( true );
}

// Function: parse_quoted_string
undefined4 parse_quoted_string(undefined4 *param_1,void **param_2) {
  undefined4 uVar1;
  undefined local_24 [12];
  int local_18;
  int local_14;
  void *local_10;
  
  local_10 = (void *)0x0;
  strio_init(local_24);
  io_getc(*param_1);
LAB_00015d0e:
  do {
    local_14 = io_getc(*param_1);
    if (local_14 < 0) {
LAB_00015edd:
      free(local_10);
      strio_free(local_24);
      return 0;
    }
    if (local_14 == 0x22) {
      local_10 = (void *)init_expression(1);
      if (local_10 != (void *)0x0) {
        uVar1 = strio_dup(local_24);
        *(undefined4 *)((int)local_10 + 0xc) = uVar1;
        if (*(int *)((int)local_10 + 0xc) != 0) {
          *param_2 = local_10;
          strio_free(local_24);
          return 1;
        }
      }
      goto LAB_00015edd;
    }
    if (local_14 != 0x5c) {
      strio_append_char(local_24,(int)(char)local_14);
      goto LAB_00015d0e;
    }
    local_18 = io_getc(*param_1);
    if (local_18 < 0) goto LAB_00015edd;
  } while (local_18 == 10);
  if (((9 < local_18) && (local_18 < 0x79)) && (0x21 < local_18)) {
    switch(local_18) {
    case 0x22:
    case 0x5c:
_L262:
      strio_append_char(local_24,(int)(char)local_18);
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x78:
      goto LAB_00015d0e;
    case 0x61:
      strio_append_char(local_24,7);
      goto LAB_00015d0e;
    case 0x62:
      strio_append_char(local_24,8);
      goto LAB_00015d0e;
    case 0x66:
      strio_append_char(local_24,0xc);
      goto LAB_00015d0e;
    case 0x6e:
      strio_append_char(local_24,10);
      goto LAB_00015d0e;
    case 0x72:
      strio_append_char(local_24,0xd);
      goto LAB_00015d0e;
    case 0x74:
      strio_append_char(local_24,9);
      goto LAB_00015d0e;
    case 0x76:
      strio_append_char(local_24,0xb);
      goto LAB_00015d0e;
    }
  }
  strio_append_char(local_24,(int)(char)local_14);
  goto _L262;
}

// Function: skip_line
void skip_line(undefined4 *param_1) {
  int iVar1;
  
  do {
    iVar1 = io_getc(*param_1);
    if (iVar1 < 0) {
      return;
    }
  } while (iVar1 != 10);
  return;
}

// Function: skip_whitespace
void skip_whitespace(undefined4 *param_1) {
  int iVar1;
  
  do {
    iVar1 = io_getc(*param_1);
    if (iVar1 < 0) {
      return;
    }
    iVar1 = isspace(iVar1);
  } while (iVar1 != 0);
  io_ungetc(*param_1);
  return;
}

// Function: is_keyword
undefined4 is_keyword(char *param_1) {
  int iVar1;
  
  iVar1 = strcmp(param_1,"if");
  if ((((((iVar1 != 0) && (iVar1 = strcmp(param_1,"while"), iVar1 != 0)) &&
        (iVar1 = strcmp(param_1,"continue"), iVar1 != 0)) &&
       ((iVar1 = strcmp(param_1,"do"), iVar1 != 0 && (iVar1 = strcmp(param_1,"for"), iVar1 != 0))))
      && ((iVar1 = strcmp(param_1,"break"), iVar1 != 0 &&
          ((iVar1 = strcmp(param_1,"next"), iVar1 != 0 &&
           (iVar1 = strcmp(param_1,"exit"), iVar1 != 0)))))) &&
     ((iVar1 = strcmp(param_1,"print"), iVar1 != 0 && (iVar1 = strcmp(param_1,"printf"), iVar1 != 0)
      ))) {
    return 0;
  }
  return 1;
}

// Function: op_to_name
char * op_to_name(undefined4 param_1) {
  char *pcVar1;
  
  switch(param_1) {
  default:
    pcVar1 = "???";
    break;
  case 1:
    pcVar1 = "CONST_STRING";
    break;
  case 2:
    pcVar1 = "CONST_INT";
    break;
  case 3:
    pcVar1 = "CONST_REGEXP";
    break;
  case 4:
    pcVar1 = "FIELD";
    break;
  case 5:
    pcVar1 = "FIELD_VAR";
    break;
  case 6:
    pcVar1 = "VAR";
    break;
  case 7:
    pcVar1 = "ASSIGN";
    break;
  case 8:
    pcVar1 = "ASSIGN_ADD";
    break;
  case 9:
    pcVar1 = "ASSIGN_SUB";
    break;
  case 10:
    pcVar1 = "ASSIGN_MUL";
    break;
  case 0xb:
    pcVar1 = "ASSIGN_DIV";
    break;
  case 0xc:
    pcVar1 = "ASSIGN_MOD";
    break;
  case 0xd:
    pcVar1 = "CONDITIONAL";
    break;
  case 0xe:
    pcVar1 = "OR";
    break;
  case 0xf:
    pcVar1 = "AND";
    break;
  case 0x10:
    pcVar1 = "MATCH";
    break;
  case 0x11:
    pcVar1 = "NOT_MATCH";
    break;
  case 0x12:
    pcVar1 = "LT";
    break;
  case 0x13:
    pcVar1 = "GT";
    break;
  case 0x14:
    pcVar1 = "LTE";
    break;
  case 0x15:
    pcVar1 = "GTE";
    break;
  case 0x16:
    pcVar1 = "EQ";
    break;
  case 0x17:
    pcVar1 = "NEQ";
    break;
  case 0x18:
    pcVar1 = "CONCAT";
    break;
  case 0x19:
    pcVar1 = "ADD";
    break;
  case 0x1a:
    pcVar1 = "SUB";
    break;
  case 0x1b:
    pcVar1 = "MUL";
    break;
  case 0x1c:
    pcVar1 = "DIV";
    break;
  case 0x1d:
    pcVar1 = "MOD";
    break;
  case 0x1e:
    pcVar1 = "NEGATE";
    break;
  case 0x1f:
    pcVar1 = "NOT";
    break;
  case 0x20:
    pcVar1 = "INC_PRE";
    break;
  case 0x21:
    pcVar1 = "INC_POST";
    break;
  case 0x22:
    pcVar1 = "DEC_PRE";
    break;
  case 0x23:
    pcVar1 = "DEC_POST";
    break;
  case 0x25:
    pcVar1 = "DEC";
    break;
  case 0x26:
    pcVar1 = "INC";
    break;
  case 0x27:
    pcVar1 = "NEGATE_OR_SUB";
  }
  return pcVar1;
}

// Function: is_unary
undefined4 is_unary(int param_1) {
  undefined4 uVar1;
  
  if ((((param_1 == 0x1e) || (param_1 == 0x1f)) || (param_1 == 0x20)) ||
     (((param_1 == 0x21 || (param_1 == 0x22)) || (param_1 == 0x23)))) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: is_binary
undefined4 is_binary(int param_1) {
  undefined4 uVar1;
  
  if (((((((param_1 == 7) || (param_1 == 8)) || (param_1 == 9)) ||
        (((param_1 == 10 || (param_1 == 0xb)) ||
         ((param_1 == 0xc || ((param_1 == 0xe || (param_1 == 0xf)))))))) ||
       ((param_1 == 0x10 || (((param_1 == 0x11 || (param_1 == 0x12)) || (param_1 == 0x13)))))) ||
      (((param_1 == 0x14 || (param_1 == 0x15)) ||
       (((param_1 == 0x16 || ((param_1 == 0x17 || (param_1 == 0x18)))) || (param_1 == 0x19)))))) ||
     ((((param_1 == 0x1a || (param_1 == 0x1b)) || (param_1 == 0x1c)) || (param_1 == 0x1d)))) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: is_assign
undefined4 is_assign(int param_1) {
  undefined4 uVar1;
  
  if ((((param_1 == 7) || (param_1 == 8)) || (param_1 == 9)) ||
     (((param_1 == 10 || (param_1 == 0xb)) || (param_1 == 0xc)))) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: is_assignable
undefined4 is_assignable(int param_1) {
  undefined4 uVar1;
  
  if (((param_1 == 4) || (param_1 == 5)) || (param_1 == 6)) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: unlink
int unlink(char *__name) {
  if (*(int *)(__name + 4) != 0) {
    *(undefined4 *)(*(int *)(__name + 4) + 8) = *(undefined4 *)(__name + 8);
  }
  if (*(int *)(__name + 8) != 0) {
    *(undefined4 *)(*(int *)(__name + 8) + 4) = *(undefined4 *)(__name + 4);
  }
  *(undefined4 *)(__name + 4) = 0;
  *(undefined4 *)(__name + 8) = 0;
  return (int)__name;
}

// Function: treeify_expression
undefined4 treeify_expression(int **param_1,int param_2) {
  int iVar1;
  int iVar2;
  int *piVar3;
  int *local_10;
  
  if (*param_1 != (int *)0x0) {
    iVar1 = (*param_1)[2];
    (*param_1)[2] = 0;
    for (local_10 = *param_1; local_10 != (int *)0x0; local_10 = (int *)local_10[1]) {
      if ((*local_10 == 0x1f) && (local_10[3] == 0)) {
        if ((local_10[1] == 0) || (iVar2 = treeify_expression(local_10 + 1,1), iVar2 == 0))
        goto LAB_00017454;
        local_10[3] = local_10[1];
        unlink((char *)local_10[3]);
      }
    }
    if (1 < param_2) {
      for (local_10 = *param_1; local_10 != (int *)0x0; local_10 = (int *)local_10[1]) {
        if ((*local_10 == 0x25) && (local_10[3] == 0)) {
          if ((local_10[2] == 0) || (iVar2 = is_assignable(*(undefined4 *)local_10[2]), iVar2 == 0))
          {
            if ((local_10[1] != 0) &&
               (iVar2 = is_assignable(*(undefined4 *)local_10[1]), iVar2 != 0)) {
              *local_10 = 0x22;
              local_10[3] = local_10[1];
              unlink((char *)local_10[3]);
            }
          }
          else {
            *local_10 = 0x23;
            local_10[3] = local_10[2];
            unlink((char *)local_10[3]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
        }
        if ((*local_10 == 0x26) && (local_10[3] == 0)) {
          if ((local_10[2] == 0) || (iVar2 = is_assignable(*(undefined4 *)local_10[2]), iVar2 == 0))
          {
            if ((local_10[1] != 0) &&
               (iVar2 = is_assignable(*(undefined4 *)local_10[1]), iVar2 != 0)) {
              *local_10 = 0x20;
              local_10[3] = local_10[1];
              unlink((char *)local_10[3]);
            }
          }
          else {
            *local_10 = 0x21;
            local_10[3] = local_10[2];
            unlink((char *)local_10[3]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
        }
      }
      if (3 < param_2) {
        for (local_10 = *param_1; local_10[1] != 0; local_10 = (int *)local_10[1]) {
        }
        for (; local_10 != (int *)0x0; local_10 = (int *)local_10[2]) {
          iVar2 = is_assign(*local_10);
          if ((iVar2 != 0) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               (((local_10[1] == 0 ||
                 (iVar2 = is_assignable(*(undefined4 *)local_10[2]), iVar2 == 0)) ||
                (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0)))) goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
        }
        for (local_10 = *param_1; local_10[1] != 0; local_10 = (int *)local_10[1]) {
        }
        for (; local_10 != (int *)0x0; local_10 = (int *)local_10[2]) {
          if ((*local_10 == 0xd) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[5] = local_10[1];
            unlink((char *)local_10[5]);
            local_10[3] = (int)*param_1;
            *(undefined4 *)(local_10[2] + 4) = 0;
            local_10[2] = 0;
            *param_1 = local_10;
            iVar2 = treeify_expression(local_10 + 3,99);
            if (iVar2 == 0) goto LAB_00017454;
          }
        }
        for (local_10 = *param_1; local_10 != (int *)0x0; local_10 = (int *)local_10[1]) {
          if ((*local_10 == 0xe) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
        }
        for (local_10 = *param_1; local_10 != (int *)0x0; local_10 = (int *)local_10[1]) {
          if ((*local_10 == 0xf) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
        }
        for (local_10 = *param_1; local_10 != (int *)0x0; local_10 = (int *)local_10[1]) {
          if ((*local_10 == 0x11) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
          if ((*local_10 == 0x10) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
        }
        for (local_10 = *param_1; local_10 != (int *)0x0; local_10 = (int *)local_10[1]) {
          if ((*local_10 == 0x16) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
          if ((*local_10 == 0x17) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
          if ((*local_10 == 0x12) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
          if ((*local_10 == 0x14) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
          if ((*local_10 == 0x13) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
          if ((*local_10 == 0x15) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
        }
        for (local_10 = *param_1; local_10 != (int *)0x0; local_10 = (int *)local_10[1]) {
          if ((*local_10 == 0x19) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
          if ((((*local_10 == 0x27) && (local_10[1] != 0)) && (local_10[2] != 0)) &&
             ((*local_10 = 0x1a, *local_10 == 0x1a && (local_10[3] == 0)))) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
        }
        for (local_10 = *param_1; local_10 != (int *)0x0; local_10 = (int *)local_10[1]) {
          if ((*local_10 == 0x1b) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
          if ((*local_10 == 0x1c) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
          if ((*local_10 == 0x1d) && (local_10[3] == 0)) {
            if ((local_10[2] == 0) ||
               ((local_10[1] == 0 || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))))
            goto LAB_00017454;
            local_10[3] = local_10[2];
            local_10[4] = local_10[1];
            unlink((char *)local_10[3]);
            unlink((char *)local_10[4]);
            if (*param_1 == (int *)local_10[3]) {
              *param_1 = local_10;
            }
          }
        }
        for (local_10 = *param_1; local_10 != (int *)0x0; local_10 = (int *)local_10[1]) {
          if (*local_10 == 0x27) {
            if ((local_10[1] == 0) || (iVar2 = treeify_expression(local_10 + 1,99), iVar2 == 0))
            goto LAB_00017454;
            *local_10 = 0x1e;
            local_10[3] = local_10[1];
            unlink((char *)local_10[3]);
          }
        }
        while ((*param_1)[1] != 0) {
          iVar2 = treeify_expression(*param_1 + 1,99);
          if (iVar2 == 0) {
LAB_00017454:
            (*param_1)[2] = iVar1;
            return 0;
          }
          piVar3 = (int *)init_expression(0x18);
          piVar3[3] = (int)*param_1;
          piVar3[4] = (*param_1)[1];
          unlink((char *)(*param_1)[1]);
          unlink((char *)*param_1);
          *param_1 = piVar3;
        }
      }
    }
    (*param_1)[2] = iVar1;
  }
  return 1;
}

// Function: verify_expression
undefined4 verify_expression(void) {
  fdprintf(2,&DAT_0001b630);
  return 1;
}

// Function: init_expression
undefined4 * init_expression(undefined4 param_1) {
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)calloc(1,0x18);
  if (puVar1 == (undefined4 *)0x0) {
    puVar1 = (undefined4 *)0x0;
  }
  else {
    *puVar1 = param_1;
  }
  return puVar1;
}

