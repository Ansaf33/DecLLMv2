// Function: main
undefined4 main(void) {
  int iVar1;
  undefined *puVar2;
  undefined4 *puVar3;
  undefined4 uStack_30;
  char *pcStack_2c;
  undefined4 uStack_28;
  undefined4 uStack_24;
  undefined auStack_20 [12];
  int local_14;
  undefined *local_10;
  
  local_10 = &stack0x00000004;
  uStack_24 = 0x11017;
  uStack_28 = 0x1f;
  pcStack_2c = "\nWelcome to Palindrome Finder\n\n";
  uStack_30 = 1;
  iVar1 = transmit_all();
  puVar3 = (undefined4 *)auStack_20;
  if (iVar1 != 0) {
    uStack_30 = 0;
    _terminate();
    puVar3 = &uStack_30;
  }
LAB_00011041:
  do {
    *(undefined4 *)((int)puVar3 + -8) = 0x25;
    *(char **)((int)puVar3 + -0xc) = "\tPlease enter a possible palindrome: ";
    *(undefined4 *)((int)puVar3 + -0x10) = 1;
    *(undefined4 *)((int)puVar3 + -0x14) = 0x11054;
    iVar1 = transmit_all();
    puVar2 = (undefined *)puVar3;
    if (iVar1 != 0) {
      puVar2 = (undefined *)((int)puVar3 + -0x10);
      *(undefined4 *)((int)puVar3 + -0x10) = 0;
      *(undefined4 *)((int)puVar3 + -0x14) = 0x11065;
      _terminate();
    }
    *(undefined4 *)(puVar2 + -4) = 0x1106a;
    local_14 = check();
    if (local_14 == -1) {
      return 0;
    }
    puVar3 = (undefined4 *)puVar2;
    if (local_14 == 0) {
      *(undefined4 *)(puVar2 + -8) = 0x21;
      *(char **)(puVar2 + -0xc) = "\t\tNope, that\'s not a palindrome\n\n";
      *(undefined4 *)(puVar2 + -0x10) = 1;
      *(undefined4 *)(puVar2 + -0x14) = 0x1108c;
      iVar1 = transmit_all();
      if (iVar1 == 0) goto LAB_00011041;
      puVar3 = (undefined4 *)(puVar2 + -0x10);
      *(undefined4 *)(puVar2 + -0x10) = 0;
      *(undefined4 *)(puVar2 + -0x14) = 0x1109d;
      _terminate();
    }
    *(undefined4 *)((int)puVar3 + -8) = 0x1d;
    *(char **)((int)puVar3 + -0xc) = "\t\tYes, that\'s a palindrome!\n\n";
    *(undefined4 *)((int)puVar3 + -0x10) = 1;
    *(undefined4 *)((int)puVar3 + -0x14) = 0x110b0;
    iVar1 = transmit_all();
    if (iVar1 != 0) {
      *(undefined4 *)((int)puVar3 + -0x10) = 0;
      *(undefined4 *)((int)puVar3 + -0x14) = 0x110c1;
      _terminate();
      return 0;
    }
  } while( true );
}

// Function: check
undefined4 check(void) {
  int iVar1;
  char local_5c [64];
  int local_1c;
  undefined4 local_18;
  uint local_14;
  int local_10;
  
  local_10 = -1;
  local_18 = 1;
  for (local_14 = 0; local_14 < 0x40; local_14 = local_14 + 1) {
    local_5c[local_14] = '\0';
  }
  iVar1 = receive_delim(0,local_5c,0x80,10);
  if (iVar1 == 0) {
    for (local_14 = 0; local_5c[local_14] != '\0'; local_14 = local_14 + 1) {
      local_10 = local_10 + 1;
    }
    local_1c = local_10;
    if (local_10 % 2 == 1) {
      local_1c = local_10 + -1;
    }
    for (local_14 = 0; (int)local_14 <= local_1c / 2; local_14 = local_14 + 1) {
      if (local_5c[local_14] != local_5c[(local_10 - local_14) + -1]) {
        local_18 = 0;
      }
    }
    if ((local_5c[0] == '^') && (iVar1 = transmit_all(1,"\n\nEASTER EGG!\n\n",0xf), iVar1 != 0)) {
      _terminate(0);
    }
  }
  else {
    local_18 = 0xffffffff;
  }
  return local_18;
}

