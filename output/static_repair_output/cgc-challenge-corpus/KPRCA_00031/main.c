// Function: hello_cmd
undefined4 hello_cmd(void) {
  fdprintf(1,"case > Hello, you.\n");
  return 0;
}

// Function: int_cmd
undefined4 int_cmd(void) {
  int iVar1;
  undefined4 *local_14;
  int local_10;
  
  local_10 = 0;
  for (local_14 = (undefined4 *)tree_to_list(ct); local_14 != (undefined4 *)0x0;
      local_14 = (undefined4 *)*local_14) {
    iVar1 = len_list(local_14[2]);
    local_10 = local_10 + iVar1;
  }
  iVar1 = num_nodes(ct);
  fdprintf(1,"case > I have an intelligence rating of: %u / %u\n",local_10,iVar1 << 3);
  return 0;
}

// Function: rev_cmd
undefined4 rev_cmd(char *param_1) {
  size_t sVar1;
  byte *__dest;
  char *pcVar2;
  undefined4 uVar3;
  byte *local_14;
  byte *local_10;
  
  sVar1 = strlen(param_1);
  __dest = (byte *)calloc(1,sVar1 + 1);
  strcpy((char *)__dest,param_1);
  pcVar2 = strchr((char *)__dest,0);
  if (pcVar2 == (char *)0x0) {
    uVar3 = 0xffffffff;
  }
  else {
    local_14 = (byte *)strchr((char *)__dest,0);
    for (local_10 = __dest; local_14 = local_14 + -1, local_10 < local_14; local_10 = local_10 + 1)
    {
      *local_10 = *local_10 ^ *local_14;
      *local_14 = *local_14 ^ *local_10;
      *local_10 = *local_10 ^ *local_14;
    }
    fdprintf(1,"case > %s\n",__dest);
    free(__dest);
    uVar3 = 0;
  }
  return uVar3;
}

// Function: pont_cmd
undefined4 pont_cmd(int param_1) {
  undefined4 uVar1;
  int local_48;
  undefined *local_44;
  undefined *local_40;
  char *local_3c;
  undefined *local_38;
  undefined *local_34;
  undefined4 local_30;
  undefined *local_2c;
  undefined4 local_28;
  char *local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  
  local_48 = 0;
  local_44 = &DAT_00016259;
  local_40 = &DAT_0001625e;
  local_3c = "symptom ";
  local_38 = &DAT_0001626a;
  local_34 = &DAT_0001626e;
  local_30 = 0;
  local_2c = &DAT_00016273;
  local_28 = 0;
  local_24 = " complex.";
  if (param_1 == 0) {
    uVar1 = 0xffffffff;
  }
  else {
    local_14 = split_words(param_1,0);
    local_18 = new_string("Hmm, I think that ");
    if ((local_14 == 0) || (local_18 == 0)) {
      uVar1 = 0xffffffff;
    }
    else {
      for (local_10 = 0; local_10 < 10; local_10 = local_10 + 1) {
        if ((&local_48)[local_10] == 0) {
          local_20 = random_element(local_14);
          if (local_20 == 0) {
            return 0xffffffff;
          }
          append_string(local_18,local_20);
        }
        else {
          append_string(local_18,(&local_48)[local_10]);
        }
      }
      local_1c = new_string(*(undefined4 *)(local_18 + 4));
      if (local_1c == 0) {
        uVar1 = 0xffffffff;
      }
      else {
        free_string(local_18);
        fdprintf(1,"molly > %s\n",*(undefined4 *)(local_1c + 4));
        free_string(local_1c);
        free_list(local_14,0);
        uVar1 = 0;
      }
    }
  }
  return uVar1;
}

// Function: part_cmd
void part_cmd(void) {
  fdprintf(1,"BYE!\n");
                    /* WARNING: Subroutine does not return */
  exit(0);
}

// Function: is_command
undefined4 is_command(char *param_1,undefined4 *param_2) {
  char **ppcVar1;
  size_t sVar2;
  int iVar3;
  undefined4 *local_10;
  
  if ((param_1 != (char *)0x0) && (sVar2 = strlen(param_1), sVar2 != 0)) {
    for (local_10 = param_2; local_10 != (undefined4 *)0x0; local_10 = (undefined4 *)*local_10) {
      ppcVar1 = (char **)local_10[2];
      sVar2 = strlen(*ppcVar1);
      iVar3 = strncmp(param_1,*ppcVar1,sVar2);
      if (iVar3 == 0) {
        return 1;
      }
    }
  }
  return 0;
}

// Function: get_command
char ** get_command(char *param_1,undefined4 *param_2) {
  char **ppcVar1;
  size_t __n;
  int iVar2;
  undefined4 *local_10;
  
  if (param_1 != (char *)0x0) {
    for (local_10 = param_2; local_10 != (undefined4 *)0x0; local_10 = (undefined4 *)*local_10) {
      ppcVar1 = (char **)local_10[2];
      __n = strlen(*ppcVar1);
      iVar2 = strncmp(param_1,*ppcVar1,__n);
      if (iVar2 == 0) {
        return ppcVar1;
      }
    }
  }
  return (char **)0x0;
}

// Function: will_chat
undefined4 will_chat(uint param_1) {
  long lVar1;
  undefined4 uVar2;
  int local_14;
  uint local_10;
  
  lVar1 = random();
  if (lVar1 < 0) {
    uVar2 = 0;
  }
  else if ((local_14 == 4) && (local_10 % 100 <= param_1)) {
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

// Function: follow_chain
undefined4 follow_chain(int param_1,uint param_2) {
  longlong lVar1;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  int local_2c;
  int local_28;
  undefined4 local_24;
  int local_20;
  uint local_1c;
  uint local_18;
  undefined8 local_14;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    local_30 = 0;
  }
  else {
    local_30 = 0;
    local_34 = 0;
    local_20 = upto_last(param_1);
    if (local_20 == 0) {
      local_30 = 0;
    }
    else {
      local_14 = key_from_wordlist(local_20);
      if (local_14 == 0) {
        local_30 = 0;
      }
      else {
        for (local_18 = 0; local_18 < param_2; local_18 = local_18 + 1) {
          local_38 = copy_list(local_20,0,0);
          local_34 = 0;
          lVar1 = local_14;
          for (local_1c = 0; local_1c < 0xf; local_1c = local_1c + 1) {
            local_14 = lVar1;
            local_24 = lindex(local_38,0);
            append_list(&local_34,local_24,1);
            local_28 = get_tree(ct,local_14);
            lVar1 = local_14;
            if (local_28 == 0) break;
            local_2c = random_element(local_28);
            if (local_2c == 0) {
              _error(3,
                     "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/main.c"
                     ,0xf4);
            }
            local_38 = copy_list(local_38,1,0);
            append_list(&local_38,local_2c,1);
            lVar1 = key_from_wordlist(local_38);
          }
          local_14 = lVar1;
          append_list(&local_30,local_34,1);
        }
      }
    }
  }
  return local_30;
}

// Function: gen_msg
int gen_msg(undefined4 *param_1) {
  uint uVar1;
  uint uVar2;
  undefined4 *local_18;
  undefined4 *local_14;
  int local_10;
  
  local_10 = 0;
  local_14 = param_1;
  do {
    if (local_14 == (undefined4 *)0x0) {
      return local_10;
    }
    for (local_18 = (undefined4 *)follow_chain(local_14[2],5); local_18 != (undefined4 *)0x0;
        local_18 = (undefined4 *)*local_18) {
      if (local_10 == 0) {
LAB_00012134:
        local_10 = local_18[2];
      }
      else {
        uVar1 = len_list(local_18[2]);
        uVar2 = len_list(local_10);
        if (uVar2 < uVar1) goto LAB_00012134;
      }
    }
    local_14 = (undefined4 *)*local_14;
  } while( true );
}

// Function: wordlist_to_string
undefined4 wordlist_to_string(undefined4 *param_1) {
  undefined4 uVar1;
  int iVar2;
  undefined4 *local_10;
  
  if (param_1 == (undefined4 *)0x0) {
    uVar1 = 0;
  }
  else {
    uVar1 = new_string(0);
    for (local_10 = param_1; local_10 != (undefined4 *)0x0; local_10 = (undefined4 *)*local_10) {
      iVar2 = append_string(uVar1,local_10[2]);
      if ((iVar2 < 0) || (iVar2 = append_string(uVar1,&DAT_00016273), iVar2 < 0)) {
        free_string(uVar1);
        return 0;
      }
    }
  }
  return uVar1;
}

// Function: tick_common
undefined4 tick_common(char *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4) {
  int iVar1;
  char **ppcVar2;
  char *pcVar3;
  size_t sVar4;
  undefined4 uVar5;
  int iVar6;
  int iVar7;
  
  iVar1 = is_command(param_1,param_2);
  if (iVar1 == 0) {
    iVar1 = str_to_wordlists(param_1);
    if (iVar1 == 0) {
      uVar5 = 0;
    }
    else {
      uVar5 = gen_msg(iVar1);
      iVar6 = wordlist_to_string(uVar5);
      if (iVar6 != 0) {
        iVar7 = will_chat(param_4);
        if (iVar7 != 0) {
          fdprintf(1,"%s > %s\n",param_3,*(undefined4 *)(iVar6 + 4));
        }
      }
      crosstalk = iVar6;
      iVar1 = insert_wordlists(iVar1,&ct);
      if (iVar1 < 0) {
        uVar5 = 0xffffffff;
      }
      else {
        uVar5 = 0;
      }
    }
  }
  else {
    ppcVar2 = (char **)get_command(param_1,param_2);
    if (ppcVar2 == (char **)0x0) {
      _error(4,
             "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/main.c"
             ,0x12a);
    }
    pcVar3 = strchr(param_1,0x20);
    if (pcVar3 == (char *)0x0) {
      sVar4 = strlen(*ppcVar2);
      param_1 = param_1 + sVar4;
    }
    else {
      param_1 = strchr(param_1,0x20);
      param_1 = param_1 + 1;
    }
    (*(code *)ppcVar2[1])(param_1);
    uVar5 = 0;
  }
  return uVar5;
}

// Function: tick_main_bot
void tick_main_bot(undefined4 param_1) {
  tick_common(param_1,b1_cmds,&DAT_00016336,0x21);
  return;
}

// Function: tick_buddy_bot
void tick_buddy_bot(undefined4 param_1) {
  tick_common(param_1,b2_cmds,"molly",10);
  return;
}

// Function: print_prompt
void print_prompt(void) {
  fdprintf(1,&DAT_00016348,"you > ");
  return;
}

// Function: load_corpus
void load_corpus(void) {
  int iVar1;
  undefined **local_10;
  
  for (local_10 = &sentences; *local_10 != (undefined *)0x0; local_10 = local_10 + 1) {
    iVar1 = split_words(*local_10,1);
    if (iVar1 == 0) {
      _error(3,
             "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/main.c"
             ,0x15b);
    }
    iVar1 = insert_wordlist(iVar1,&ct);
    if (iVar1 < 0) {
      _error(3,
             "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/main.c"
             ,0x15e);
    }
  }
  return;
}

// Function: init
int init(EVP_PKEY_CTX *ctx) {
  int iVar1;
  undefined8 uVar2;
  
  append_list(&b1_cmds,&hello,0);
  append_list(&b1_cmds,&_int,0);
  append_list(&b1_cmds,&rev,0);
  append_list(&b1_cmds,&part,0);
  append_list(&b2_cmds,&pont,0);
  uVar2 = hash_str("ROOT NODE");
  ct = init_tree(uVar2,"ROOT NODE");
  crosstalk = 0;
  load_corpus();
  iVar1 = fdprintf(1,&DAT_00016348,
                   "\nWelcome to the Enslavednode chat server!\nThe bots here love to talk, but they are shy...\nYou need to get the ball rolling.\n"
                  );
  return iVar1;
}

// Function: main
undefined4 main(void) {
  int iVar1;
  EVP_PKEY_CTX *in_stack_ffffffe0;
  int local_18;
  void *local_14;
  undefined *local_10;
  
  local_10 = &stack0x00000004;
  local_14 = (void *)0x0;
  init(in_stack_ffffffe0);
  while( true ) {
    while( true ) {
      local_14 = calloc(1,0x1fbf);
      if (local_14 == (void *)0x0) {
        _error(1,
               "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/main.c"
               ,0x184);
      }
      print_prompt();
      memset(local_14,0,0x1fbf);
      iVar1 = readline(1,local_14,0x1fbe,&local_18);
      if ((-1 < iVar1) && (local_18 != 0)) break;
      free(local_14);
    }
    *(undefined *)((int)local_14 + local_18) = 0;
    iVar1 = tick_main_bot(local_14);
    if (iVar1 < 0) break;
    if ((crosstalk == 0) || (*(int *)(crosstalk + 4) == 0)) {
      free(local_14);
    }
    else {
      tick_buddy_bot(*(undefined4 *)(crosstalk + 4));
      crosstalk = 0;
      free(local_14);
    }
  }
  free(local_14);
  return 0;
}

