// Function: InitFS
undefined4 InitFS(uint param_1) {
  char *pcVar1;
  undefined4 uVar2;
  int iVar3;
  
  if ((param_1 & 0xfff) != 0) {
    param_1 = (param_1 & 0xfffff000) + 0x1000;
  }
  if (param_1 < 0x100001) {
    if ((param_1 & 0x1ff) == 0) {
      bzero(&fs,0x10c);
      fs = 0x43524653;
      DAT_0001a0c4 = 0x200;
      DAT_0001a0c8 = param_1;
      iVar3 = allocate(0x1000,0,&DAT_0001a0cc);
      if (iVar3 == 0) {
        bzero(DAT_0001a0cc,0x1000);
        bzero(&free_list,0x800);
        iVar3 = allocate(0x1000,0,&DAT_0001a1e4);
        if (iVar3 == 0) {
          bzero(_DAT_0001a1e4,0x1000);
          pcVar1 = DAT_0001a0cc;
          strcpy(DAT_0001a0cc,"/");
          *(undefined4 *)(pcVar1 + 0xd0) = 2;
          *(undefined4 *)(pcVar1 + 0xd4) = 0;
          *(undefined4 *)(pcVar1 + 0xd8) = 1;
          *(void **)(pcVar1 + 0xe0) = _DAT_0001a1e4;
          free_list = 0x80;
          uVar2 = 0;
        }
        else {
          puts("Failed to allocate data blocks\n");
          uVar2 = 0xffffffff;
        }
      }
      else {
        puts("Failed to allocate inode page\n");
        uVar2 = 0xffffffff;
      }
    }
    else {
      puts("Requested filesystem size is not an even number of blocks\n");
      uVar2 = 0xffffffff;
    }
  }
  else {
    puts("Requested filesystem size is too large\n");
    uVar2 = 0xffffffff;
  }
  return uVar2;
}

// Function: DestroyFS
undefined4 DestroyFS(void) {
  uint uVar1;
  uint local_10;
  
  uVar1 = DAT_0001a0c8 >> 0xc;
  for (local_10 = 0; local_10 < uVar1; local_10 = local_10 + 1) {
    if (*(int *)(&DAT_0001a1e4 + local_10 * 8) != 0) {
      deallocate(&DAT_0001a1e4 + local_10 * 8,0x1000);
    }
  }
  for (local_10 = 0; (int)local_10 < 0x40; local_10 = local_10 + 1) {
    if ((&DAT_0001a0cc)[local_10] != 0) {
      deallocate(&DAT_0001a0cc + local_10,0x1000);
    }
  }
  return 0;
}

// Function: FindInode
int FindInode(char *param_1) {
  int iVar1;
  int iVar2;
  int local_14;
  int local_10;
  
  if (param_1 != (char *)0x0) {
    for (local_10 = 0; (local_10 < 0x40 && ((&DAT_0001a0cc)[local_10] != 0));
        local_10 = local_10 + 1) {
      iVar1 = (&DAT_0001a0cc)[local_10];
      for (local_14 = 0; local_14 < 0x10; local_14 = local_14 + 1) {
        if (((*(int *)(iVar1 + local_14 * 0x100 + 0xd0) == 1) ||
            (*(int *)(iVar1 + local_14 * 0x100 + 0xd0) == 2)) &&
           (iVar2 = strcmp((char *)(iVar1 + local_14 * 0x100),param_1), iVar2 == 0)) {
          return iVar1 + local_14 * 0x100;
        }
      }
    }
  }
  return 0;
}

// Function: FindDirEntry
char * FindDirEntry(int param_1,char *param_2) {
  int iVar1;
  char *__s1;
  int iVar2;
  int local_14;
  int local_10;
  
  if (((param_1 != 0) && (param_2 != (char *)0x0)) && (*(int *)(param_1 + 0xd0) == 2)) {
    for (local_10 = 0; local_10 < 8; local_10 = local_10 + 1) {
      iVar1 = *(int *)(param_1 + (local_10 + 0x38) * 4);
      if (iVar1 != 0) {
        for (local_14 = 0;
            (local_14 < 0x81 && ((uint)((local_14 + 1) * 4) <= *(uint *)(param_1 + 0xd4)));
            local_14 = local_14 + 1) {
          __s1 = *(char **)(iVar1 + local_14 * 4);
          if ((__s1 != (char *)0x0) && (iVar2 = strcmp(__s1,param_2), iVar2 == 0)) {
            return __s1;
          }
        }
      }
    }
  }
  return (char *)0x0;
}

// Function: CheckFileExists
undefined4 CheckFileExists(char *param_1,int *param_2) {
  int iVar1;
  char *pcVar2;
  undefined4 local_18;
  int local_14;
  char *local_10;
  
  local_10 = param_1;
  local_18 = 1;
  if (param_1 == (char *)0x0) {
    local_18 = 0xffffffff;
  }
  else if (*param_1 == '/') {
    local_14 = FindInode(&DAT_000172bc);
    if (local_14 == 0) {
      puts("failed to root find inode");
      local_18 = 0xffffffff;
    }
    else {
      iVar1 = strcmp(param_1,"/");
      if (iVar1 == 0) {
        *param_2 = local_14;
        local_18 = 2;
      }
      else {
        while (pcVar2 = strtok(local_10,"/"), pcVar2 != (char *)0x0) {
          local_10 = (char *)0x0;
          local_18 = 0;
          iVar1 = FindDirEntry(local_14,pcVar2);
          if (iVar1 != 0) {
            if (*(int *)(iVar1 + 0xd0) != 2) {
              *param_2 = iVar1;
              local_18 = 1;
              break;
            }
            *param_2 = iVar1;
            local_18 = 2;
            local_14 = iVar1;
          }
        }
        if (local_10 != (char *)0x0) {
          local_18 = 0;
        }
      }
    }
  }
  else {
    puts("invalid filename");
    local_18 = 0xffffffff;
  }
  return local_18;
}

// Function: FindFreeInode
int FindFreeInode(void) {
  int iVar1;
  int local_18;
  int local_14;
  int local_10;
  
  local_10 = 0;
  do {
    if (0x3f < local_10) {
      return 0;
    }
    local_18 = (&DAT_0001a0cc)[local_10];
    if (local_18 == 0) {
      iVar1 = allocate(0x1000,0,&DAT_0001a0cc + local_10);
      if (iVar1 != 0) {
        puts("Failed to allocate inode page\n");
        return 0;
      }
      bzero((void *)(&DAT_0001a0cc)[local_10],0x1000);
      local_18 = (&DAT_0001a0cc)[local_10];
    }
    for (local_14 = 0; local_14 < 0x10; local_14 = local_14 + 1) {
      iVar1 = local_18 + local_14 * 0x100;
      if (*(int *)(iVar1 + 0xd0) == 0) {
        return iVar1;
      }
    }
    local_10 = local_10 + 1;
  } while( true );
}

// Function: FindFreeDataBlock
int FindFreeDataBlock(void) {
  int iVar1;
  byte local_15;
  int local_14;
  int local_10;
  
  local_14 = 7;
  local_10 = 0;
  do {
    if (0xff < local_10) {
      return 0;
    }
    if (*(int *)(&DAT_0001a1e4 + local_10 * 8) == 0) {
      if (DAT_0001a0c8 <= (uint)(local_10 << 0xc)) {
        return 0;
      }
      iVar1 = allocate(0x1000,0,&DAT_0001a1e4 + local_10 * 8);
      if (iVar1 != 0) {
        puts("Failed to allocate data blocks\n");
        return 0;
      }
      bzero(*(void **)(&DAT_0001a1e4 + local_10 * 8),0x1000);
    }
    if ((&free_list)[local_10 * 8] != -1) {
      local_15 = (&free_list)[local_10 * 8];
      for (; ((local_15 & 1) != 0 && (local_14 != 0)); local_14 = local_14 + -1) {
        local_15 = local_15 >> 1;
      }
      (&free_list)[local_10 * 8] =
           (byte)(0x80 >> ((byte)local_14 & 0x1f)) | (&free_list)[local_10 * 8];
      return *(int *)(&DAT_0001a1e4 + local_10 * 8) + local_14 * 0x200;
    }
    local_10 = local_10 + 1;
  } while( true );
}

// Function: SplitPath
undefined4 SplitPath(char *param_1,char *param_2,char *param_3) {
  size_t local_10;
  
  if (((param_1 != (char *)0x0) && (param_2 != (char *)0x0)) && (param_3 != (char *)0x0)) {
    strcpy(param_2,param_1);
    for (local_10 = strlen(param_2); 0 < (int)local_10; local_10 = local_10 - 1) {
      if (param_2[local_10] == '/') {
        strcpy(param_3,param_2 + local_10 + 1);
        param_2[local_10] = '\0';
        break;
      }
    }
    if (local_10 == 0) {
      strcpy(param_3,param_1 + 1);
      param_2[1] = '\0';
    }
  }
  return 0;
}

// Function: CreateFile
char * CreateFile(int param_1,char *param_2) {
  char *__s;
  size_t sVar1;
  int iVar2;
  char local_240 [256];
  undefined local_140 [256];
  int local_40;
  undefined4 local_3c;
  char *local_38;
  size_t local_34;
  void *local_30;
  int local_2c;
  char *local_28;
  char *local_24;
  char *local_20;
  uint local_1c;
  uint local_18;
  int local_14;
  uint local_10;
  
  local_3c = 0;
  local_40 = 0;
  local_20 = (char *)0x0;
  local_24 = (char *)0x0;
  if ((param_1 == 0) || (param_2 == (char *)0x0)) {
    local_24 = (char *)0x0;
  }
  else {
    bzero(local_140,0x100);
    bzero(local_240,0x100);
    sVar1 = strlen(local_240);
    if (sVar1 < 0xd0) {
      local_10 = CheckFileExists(param_1,&local_3c);
      if (local_10 == -1) {
        local_24 = (char *)0x0;
      }
      else if (local_10 == 1) {
        puts("file exists");
        local_24 = (char *)0x0;
      }
      else {
        SplitPath(param_1,local_140,local_240);
        iVar2 = CheckFileExists(local_140,&local_40);
        if (iVar2 == 2) {
          sVar1 = strlen(param_2);
          local_18 = sVar1 / DAT_0001a0c4;
          sVar1 = strlen(param_2);
          if (sVar1 % DAT_0001a0c4 != 0) {
            local_18 = local_18 + 1;
          }
          if (local_18 == 0) {
            local_18 = 1;
          }
          local_1c = local_18 >> 3;
          if ((local_18 & 7) != 0) {
            local_1c = local_1c + 1;
          }
          for (local_10 = 0; local_10 < local_1c; local_10 = local_10 + 1) {
            local_38 = (char *)FindFreeInode();
            if (local_38 == (char *)0x0) {
              puts("out of inodes");
              return (char *)0x0;
            }
            if (local_10 == 0) {
              local_24 = local_38;
            }
            *(undefined4 *)(local_38 + 0xd0) = 1;
            sVar1 = strlen(param_2);
            *(size_t *)(local_38 + 0xd4) = sVar1;
            *(uint *)(local_38 + 0xd8) = local_18;
            if (local_20 != (char *)0x0) {
              *(char **)(local_20 + 0xdc) = local_38;
            }
            local_20 = local_38;
            strncpy(local_38,local_240,0xcf);
          }
          local_28 = local_24;
          local_14 = 0;
          for (local_10 = 0; local_10 < local_18; local_10 = local_10 + 1) {
            local_30 = (void *)FindFreeDataBlock();
            if (local_30 == (void *)0x0) {
              puts("out of space");
              while (__s = local_28, local_28 != (char *)0x0) {
                local_20 = local_28;
                local_28 = *(char **)(local_28 + 0xdc);
                bzero(__s,0x100);
              }
              return (char *)0x0;
            }
            if ((local_10 != 0) && ((local_10 & 7) == 0)) {
              local_28 = *(char **)(local_28 + 0xdc);
              local_14 = 0;
            }
            *(void **)(local_28 + (local_14 + 0x38) * 4) = local_30;
            local_34 = strlen(param_2 + local_10 * 0x200);
            sVar1 = local_34;
            if (0x200 < (int)local_34) {
              sVar1 = 0x200;
            }
            memcpy(local_30,param_2 + local_10 * 0x200,sVar1);
            local_14 = local_14 + 1;
          }
          local_2c = *(int *)(local_40 + 0xe0);
          for (local_10 = 0; (int)local_10 < 0x80; local_10 = local_10 + 1) {
            if (*(int *)(local_2c + local_10 * 4) == 0) {
              *(char **)(local_2c + local_10 * 4) = local_24;
              if (*(uint *)(local_40 + 0xd4) < (local_10 + 1) * 4) {
                *(uint *)(local_40 + 0xd4) = (local_10 + 1) * 4;
              }
              break;
            }
          }
          if (local_10 == 0x80) {
            local_28 = local_24;
            while (local_28 != (char *)0x0) {
              local_20 = local_28;
              local_28 = *(char **)(local_28 + 0xdc);
              for (local_14 = 0; local_14 < 8; local_14 = local_14 + 1) {
                FreeDataBlock(*(undefined4 *)(local_20 + (local_14 + 0x38) * 4));
              }
              bzero(local_20,0x100);
            }
            puts("directory is full");
            local_24 = (char *)0x0;
          }
          else {
            puts("file created");
          }
        }
        else {
          puts("directory doesn\'t exist");
          local_24 = (char *)0x0;
        }
      }
    }
    else {
      puts("file name too large");
      local_24 = (char *)0x0;
    }
  }
  return local_24;
}

// Function: ReadFile
undefined4 ReadFile(undefined4 param_1) {
  undefined4 uVar1;
  size_t in_stack_fffffdcc;
  int local_20;
  int local_1c;
  void *local_18;
  int local_14;
  uint local_10;
  
  local_20 = 0;
  local_10 = CheckFileExists(param_1,&local_20);
  if (local_10 == -1) {
    uVar1 = 1;
  }
  else if (local_10 == 0) {
    puts("file doesn\'t exist");
    uVar1 = 1;
  }
  else if (local_10 == 2) {
    puts("file is a directory");
    uVar1 = 1;
  }
  else {
    local_14 = 0;
    local_18 = *(void **)(local_20 + 0xd4);
    for (local_10 = 0; local_10 < *(uint *)(local_20 + 0xd8); local_10 = local_10 + 1) {
      if ((local_10 != 0) && ((local_10 & 7) == 0)) {
        local_20 = *(int *)(local_20 + 0xdc);
        if (local_20 == 0) {
          return 0;
        }
        local_14 = 0;
      }
      local_1c = *(int *)(local_20 + (local_14 + 0x38) * 4);
      if (local_1c == 0) {
        return 0;
      }
      if (local_18 < (void *)0x200) {
        write(local_1c,local_18,in_stack_fffffdcc);
      }
      else {
        write(local_1c,(void *)0x200,in_stack_fffffdcc);
        local_18 = (void *)((int)local_18 + -0x200);
      }
      local_14 = local_14 + 1;
    }
    puts("");
    uVar1 = 0;
  }
  return uVar1;
}

// Function: mkdir
int mkdir(char *__path,__mode_t __mode) {
  int iVar1;
  size_t sVar2;
  char local_22c [256];
  undefined local_12c [256];
  int local_2c;
  undefined local_28 [4];
  char *local_24;
  int local_20;
  int local_1c;
  int local_18;
  char *local_14;
  int local_10;
  
  if (__path == (char *)0x0) {
    iVar1 = -1;
  }
  else {
    bzero(local_12c,0x100);
    bzero(local_22c,0x100);
    local_10 = CheckFileExists(__path,local_28);
    if (local_10 == -1) {
      iVar1 = -1;
    }
    else if (local_10 == 1) {
      puts("file exists");
      iVar1 = -1;
    }
    else if (local_10 == 2) {
      puts("directory exists");
      iVar1 = -1;
    }
    else {
      SplitPath(__path,local_12c,local_22c);
      sVar2 = strlen(local_22c);
      if (sVar2 < 0xd0) {
        iVar1 = CheckFileExists(local_12c,&local_2c);
        if (iVar1 == 2) {
          local_1c = FindFreeDataBlock();
          if (local_1c == 0) {
            puts("out of space");
            iVar1 = -1;
          }
          else {
            local_14 = (char *)FindFreeInode();
            if (local_14 == (char *)0x0) {
              puts("out of inodes");
              iVar1 = -1;
            }
            else {
              *(undefined4 *)(local_14 + 0xd0) = 2;
              *(undefined4 *)(local_14 + 0xd4) = 0;
              *(undefined4 *)(local_14 + 0xd8) = 1;
              *(undefined4 *)(local_14 + 0xdc) = 0;
              strncpy(local_14,local_22c,0xcf);
              *(int *)(local_14 + 0xe0) = local_1c;
              local_20 = *(int *)(local_2c + 0xe0);
              for (local_10 = 0; local_10 < 0x81; local_10 = local_10 + 1) {
                if (*(int *)(local_20 + local_10 * 4) == 0) {
                  *(char **)(local_20 + local_10 * 4) = local_14;
                  if (*(uint *)(local_2c + 0xd4) < (uint)((local_10 + 1) * 4)) {
                    *(int *)(local_2c + 0xd4) = (local_10 + 1) * 4;
                  }
                  break;
                }
              }
              if (local_10 < 0x81) {
                puts("directory created");
                iVar1 = 0;
              }
              else {
                while (local_14 != (char *)0x0) {
                  local_24 = local_14;
                  local_14 = *(char **)(local_14 + 0xdc);
                  for (local_18 = 0; local_18 < 8; local_18 = local_18 + 1) {
                    FreeDataBlock(*(undefined4 *)(local_24 + (local_18 + 0x38) * 4));
                  }
                  bzero(local_24,0x100);
                }
                puts("directory is full");
                iVar1 = -1;
              }
            }
          }
        }
        else {
          puts("parent directory doesn\'t exist");
          iVar1 = -1;
        }
      }
      else {
        puts("file name too large");
        iVar1 = -1;
      }
    }
  }
  return iVar1;
}

// Function: rmdir
int rmdir(char *__path) {
  int iVar1;
  undefined local_228 [256];
  undefined local_128 [256];
  int local_28;
  void *local_24;
  void *local_20;
  int local_1c;
  void *local_18;
  int local_14;
  uint local_10;
  
  if (__path == (char *)0x0) {
    iVar1 = -1;
  }
  else {
    bzero(local_128,0x100);
    bzero(local_228,0x100);
    local_10 = CheckFileExists(__path,&local_24);
    if ((local_10 == 0) || (local_10 == -1)) {
      puts("directory does not exist");
      iVar1 = -1;
    }
    else if (*(int *)((int)local_24 + 0xd0) == 2) {
      iVar1 = strcmp(__path,"/");
      if (iVar1 == 0) {
        puts("unable to remove /");
        iVar1 = -1;
      }
      else {
        local_1c = *(int *)((int)local_24 + 0xe0);
        for (local_10 = 0; (int)local_10 < 0x80; local_10 = local_10 + 1) {
          if (*(int *)(local_1c + local_10 * 4) != 0) {
            puts("directory not empty");
            return -1;
          }
        }
        SplitPath(__path,local_128,local_228);
        iVar1 = CheckFileExists(local_128,&local_28);
        if (iVar1 == 2) {
          local_1c = *(int *)(local_28 + 0xe0);
          for (local_10 = 0; (int)local_10 < 0x80; local_10 = local_10 + 1) {
            if (*(void **)(local_1c + local_10 * 4) == local_24) {
              *(undefined4 *)(local_1c + local_10 * 4) = 0;
              if ((local_10 + 1) * 4 == *(int *)(local_28 + 0xd4)) {
                *(uint *)(local_28 + 0xd4) = local_10 * 4;
              }
              break;
            }
          }
          if (local_10 == 0x80) {
            puts(
                "parent directory doesn\'t have an entry for this directory...file system corruption detected"
                );
            iVar1 = 1;
          }
          else {
            local_14 = 0;
            local_18 = local_24;
            for (local_10 = 0; local_10 < *(uint *)((int)local_18 + 0xd8); local_10 = local_10 + 1)
            {
              if ((local_10 != 0) && ((local_10 & 7) == 0)) {
                local_20 = *(void **)((int)local_18 + 0xdc);
                bzero(local_18,0x100);
                local_18 = local_20;
                local_14 = 0;
                if (local_20 == (void *)0x0) break;
              }
              if (*(int *)((int)local_18 + (local_14 + 0x38) * 4) != 0) {
                FreeDataBlock(*(undefined4 *)((int)local_18 + (local_14 + 0x38) * 4));
              }
              local_14 = local_14 + 1;
            }
            bzero(local_24,0x100);
            puts("directory removed");
            iVar1 = 0;
          }
        }
        else {
          puts("parent directory doesn\'t exist...file system corruption detected");
          iVar1 = 1;
        }
      }
    }
    else {
      puts("not a directory");
      iVar1 = -1;
    }
  }
  return iVar1;
}

// Function: FreeDataBlock
bool FreeDataBlock(uint param_1) {
  int iVar1;
  int local_14;
  int local_10;
  
  if (param_1 == 0) {
    return true;
  }
  for (local_10 = 0; local_10 < 0x100; local_10 = local_10 + 1) {
    iVar1 = local_10 * 8;
    if ((*(uint *)(&DAT_0001a1e4 + iVar1) <= param_1) &&
       (param_1 <= *(int *)(&DAT_0001a1e4 + iVar1) + 0xe00U)) {
      local_14 = 0;
      goto LAB_0001365b;
    }
  }
LAB_00013694:
  if (local_10 == 0x100) {
    puts("unable to free block");
  }
  return local_10 == 0x100;
LAB_0001365b:
  if (7 < local_14) goto LAB_00013661;
  if (param_1 == *(int *)(&DAT_0001a1e4 + iVar1) + local_14 * 0x200) {
    (&free_list)[iVar1] = (byte)(0x80 >> ((byte)local_14 & 0x1f)) ^ (&free_list)[iVar1];
    bzero((void *)(*(int *)(&DAT_0001a1e4 + iVar1) + local_14 * 0x200),0x200);
    goto LAB_00013661;
  }
  local_14 = local_14 + 1;
  goto LAB_0001365b;
LAB_00013661:
  if (local_14 == 8) {
    puts("unable to free block");
    return true;
  }
  goto LAB_00013694;
}

// Function: unlink
int unlink(char *__name) {
  int iVar1;
  undefined local_228 [256];
  undefined local_128 [256];
  int local_28;
  void *local_24;
  int local_20;
  void *local_1c;
  void *local_18;
  int local_14;
  uint local_10;
  
  bzero(local_128,0x100);
  bzero(local_228,0x100);
  local_10 = CheckFileExists(__name,&local_24);
  if (local_10 == -1) {
    iVar1 = 1;
  }
  else if (local_10 == 0) {
    puts("file doesn\'t exist");
    iVar1 = 1;
  }
  else if (local_10 == 2) {
    puts("file is a directory");
    iVar1 = 1;
  }
  else {
    SplitPath(__name,local_128,local_228);
    iVar1 = CheckFileExists(local_128,&local_28);
    if (iVar1 == 2) {
      local_14 = 0;
      local_18 = local_24;
      for (local_10 = 0; local_10 < *(uint *)((int)local_18 + 0xd8); local_10 = local_10 + 1) {
        if ((local_10 != 0) && ((local_10 & 7) == 0)) {
          local_1c = *(void **)((int)local_18 + 0xdc);
          bzero(local_18,0x100);
          local_18 = local_1c;
          local_14 = 0;
          if (local_1c == (void *)0x0) break;
        }
        if (*(int *)((int)local_18 + (local_14 + 0x38) * 4) != 0) {
          FreeDataBlock(*(undefined4 *)((int)local_18 + (local_14 + 0x38) * 4));
        }
        local_14 = local_14 + 1;
      }
      local_20 = *(int *)(local_28 + 0xe0);
      for (local_10 = 0; (int)local_10 < 0x80; local_10 = local_10 + 1) {
        if (*(void **)(local_20 + local_10 * 4) == local_24) {
          *(undefined4 *)(local_20 + local_10 * 4) = 0;
          if ((local_10 + 1) * 4 == *(int *)(local_28 + 0xd4)) {
            *(uint *)(local_28 + 0xd4) = local_10 * 4;
          }
          break;
        }
      }
      bzero(local_24,0x100);
      puts("file removed");
      iVar1 = 0;
    }
    else {
      puts("parent directory doesn\'t exist...file system corruption detected");
      iVar1 = 1;
    }
  }
  return iVar1;
}

// Function: fopen
FILE * fopen(char *__filename,char *__modes) {
  uint uVar1;
  int iVar2;
  FILE *local_24;
  char *local_20;
  void *local_1c;
  void *local_18;
  uint local_14;
  int local_10;
  
  local_20 = (char *)0x0;
  if (__filename == (char *)0x0) {
    local_24 = (FILE *)0x0;
  }
  else {
    local_10 = CheckFileExists(__filename,&local_20);
    if (local_10 == -1) {
      local_24 = (FILE *)0x0;
    }
    else {
      if (local_10 == 0) {
        iVar2 = strcmp(__modes,"w");
        if (iVar2 != 0) {
          puts("file doesn\'t exist");
          return (FILE *)0x0;
        }
        local_20 = (char *)CreateFile(__filename,&DAT_00017382);
        if (local_20 == (char *)0x0) {
          puts("file creation failed\n");
          return (FILE *)0x0;
        }
      }
      else if (local_10 == 2) {
        puts("file is a directory");
        return (FILE *)0x0;
      }
      iVar2 = allocate(0x18,0,&local_24);
      if (iVar2 == 0) {
        bzero(local_24,0x18);
        local_24->_flags = (int)local_20;
        iVar2 = strcmp(__modes,"r");
        if (iVar2 == 0) {
          local_24->_IO_read_ptr = (char *)0x0;
          local_24->_IO_read_end = (char *)0x0;
          local_24->_IO_read_base = local_20;
        }
        else {
          iVar2 = strcmp(__modes,"w");
          if (iVar2 == 0) {
            for (local_10 = 1; local_10 < 8; local_10 = local_10 + 1) {
              FreeDataBlock(*(undefined4 *)(local_20 + (local_10 + 0x38) * 4));
              *(undefined4 *)(local_20 + (local_10 + 0x38) * 4) = 0;
            }
            local_18 = *(void **)(local_20 + 0xdc);
            while (local_18 != (void *)0x0) {
              for (local_10 = 0; local_10 < 8; local_10 = local_10 + 1) {
                FreeDataBlock(*(undefined4 *)((int)local_18 + (local_10 + 0x38) * 4));
                *(undefined4 *)((int)local_18 + (local_10 + 0x38) * 4) = 0;
              }
              local_1c = *(void **)((int)local_18 + 0xdc);
              bzero(local_18,0x100);
              local_18 = local_1c;
            }
            bzero(*(void **)(local_20 + 0xe0),0x200);
            *(undefined4 *)(local_20 + 0xd4) = 0;
            local_24->_IO_read_ptr = (char *)0x0;
            local_24->_IO_read_end = (char *)0x1;
            local_24->_IO_read_base = local_20;
          }
          else {
            iVar2 = strcmp(__modes,"a");
            if (iVar2 == 0) {
              local_24->_IO_read_ptr = *(char **)(local_20 + 0xd4);
              local_24->_IO_read_end = (char *)0x2;
              local_24->_IO_read_base = local_20;
              uVar1 = *(uint *)(local_20 + 0xd4);
              local_24->_IO_write_base = (char *)((*(uint *)(local_20 + 0xd4) & 0xfff) >> 9);
              local_24->_IO_write_ptr = (char *)(*(uint *)(local_20 + 0xd4) & 0x1ff);
              local_14 = uVar1 >> 0xc;
              while (local_14 != 0) {
                local_24->_IO_read_base = *(char **)(local_24->_IO_read_base + 0xdc);
                local_14 = local_14 - 1;
              }
            }
            else {
              deallocate(local_24,0x18);
              local_24 = (FILE *)0x0;
            }
          }
        }
      }
      else {
        puts("Failed to allocate FILE struct\n");
        local_24 = (FILE *)0x0;
      }
    }
  }
  return local_24;
}

// Function: fread
size_t fread(void *__ptr,size_t __size,size_t __n,FILE *__stream) {
  char *local_18;
  int local_14;
  char *local_10;
  char *local_c;
  uint local_8;
  
  if ((__ptr == (void *)0x0) || (__stream == (FILE *)0x0)) {
    local_8 = 0;
  }
  else if ((__stream->_IO_read_end == (char *)0x0) && (__stream->_flags != 0)) {
    local_18 = __stream->_IO_read_base;
    local_c = __stream->_IO_write_base;
    local_14 = *(int *)(local_18 + (int)(local_c + 0x38) * 4);
    local_8 = 0;
    local_10 = __stream->_IO_write_ptr;
    while ((local_8 < __size * __n && (__stream->_IO_read_ptr < *(char **)(__stream->_flags + 0xd4))
           )) {
      if ((local_10 != (char *)0x0) && (((uint)local_10 & 0x1ff) == 0)) {
        if ((local_c != (char *)0x0) && (((uint)local_c & 7) == 0)) {
          local_18 = *(char **)(local_18 + 0xdc);
          if (local_18 == (char *)0x0) {
            __stream->_IO_read_base = (char *)0x0;
            __stream->_IO_write_base = local_c;
            __stream->_IO_write_ptr = local_10;
            return local_8;
          }
          local_c = (char *)0xffffffff;
        }
        local_14 = *(int *)(local_18 + (int)(local_c + 0x39) * 4);
        if (local_14 == 0) {
          __stream->_IO_read_base = local_18;
          __stream->_IO_write_base = local_c + 1;
          __stream->_IO_write_ptr = local_10;
          return local_8;
        }
        local_10 = (char *)0x0;
        local_c = local_c + 1;
      }
      *(char *)(local_8 + (int)__ptr) = local_10[local_14];
      __stream->_IO_read_ptr = __stream->_IO_read_ptr + 1;
      local_8 = local_8 + 1;
      local_10 = local_10 + 1;
    }
    __stream->_IO_read_base = local_18;
    __stream->_IO_write_base = local_c;
    __stream->_IO_write_ptr = local_10;
  }
  else {
    local_8 = 0;
  }
  return local_8;
}

// Function: fwrite
size_t fwrite(void *__ptr,size_t __size,size_t __n,FILE *__s) {
  char *pcVar1;
  int local_20;
  char *local_1c;
  char *local_18;
  char *local_14;
  uint local_10;
  
  if ((__ptr == (void *)0x0) || (__s == (FILE *)0x0)) {
    local_10 = 0;
  }
  else if ((__s->_IO_read_end == (char *)0x1) || (__s->_IO_read_end == (char *)0x2)) {
    local_14 = __s->_IO_read_base;
    local_18 = __s->_IO_write_base;
    local_20 = *(int *)(local_14 + (int)(local_18 + 0x38) * 4);
    local_1c = __s->_IO_write_ptr;
    for (local_10 = 0; local_10 < __size * __n; local_10 = local_10 + 1) {
      if ((local_1c != (char *)0x0) && (((uint)local_1c & 0x1ff) == 0)) {
        local_18 = local_18 + 1;
        if ((local_18 != (char *)0x0) && (((uint)local_18 & 7) == 0)) {
          if (*(int *)(local_14 + 0xdc) != 0) {
            __s->_IO_read_base = local_14;
            __s->_IO_write_base = local_18;
            __s->_IO_write_ptr = local_1c;
            return local_10;
          }
          pcVar1 = (char *)FindFreeInode();
          if (pcVar1 == (char *)0x0) {
            puts("out of inodes");
            return 0;
          }
          *(undefined4 *)(pcVar1 + 0xd0) = *(undefined4 *)(__s->_flags + 0xd0);
          *(undefined4 *)(pcVar1 + 0xd4) = *(undefined4 *)(__s->_flags + 0xd4);
          *(undefined4 *)(pcVar1 + 0xd8) = *(undefined4 *)(__s->_flags + 0xd8);
          *(undefined4 *)(pcVar1 + 0xdc) = 0;
          strncpy(pcVar1,(char *)__s->_flags,0xcf);
          *(char **)(local_14 + 0xdc) = pcVar1;
          local_18 = (char *)0x0;
        }
        local_20 = FindFreeDataBlock();
        if (local_20 == 0) {
          puts("out of space");
          __s->_IO_read_base = local_14;
          __s->_IO_write_base = local_18;
          __s->_IO_write_ptr = local_1c;
          while (local_14 != (char *)0x0) {
            pcVar1 = *(char **)(local_14 + 0xdc);
            bzero(local_14,0x100);
            local_14 = pcVar1;
          }
          return local_10;
        }
        *(int *)(local_14 + (int)(local_18 + 0x38) * 4) = local_20;
        local_1c = (char *)0x0;
        *(int *)(__s->_flags + 0xd8) = *(int *)(__s->_flags + 0xd8) + 1;
      }
      local_1c[local_20] = *(char *)(local_10 + (int)__ptr);
      *(int *)(__s->_flags + 0xd4) = *(int *)(__s->_flags + 0xd4) + 1;
      __s->_IO_read_ptr = __s->_IO_read_ptr + 1;
      local_1c = local_1c + 1;
    }
    __s->_IO_read_base = local_14;
    __s->_IO_write_base = local_18;
    __s->_IO_write_ptr = local_1c;
  }
  else {
    local_10 = 0;
  }
  return local_10;
}

// Function: fclose
int fclose(FILE *__stream) {
  int iVar1;
  
  if (__stream == (FILE *)0x0) {
    iVar1 = -1;
  }
  else {
    deallocate(__stream,0x18);
    iVar1 = 0;
  }
  return iVar1;
}

// Function: StatusFS
undefined4 StatusFS(void) {
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  
  local_18 = 0;
  local_1c = 0;
  printf("Filesystem info:\n");
  printf("  Blocksize: @d\n",DAT_0001a0c4);
  for (local_10 = 0; (local_10 < 0x40 && ((&DAT_0001a0cc)[local_10] != 0)); local_10 = local_10 + 1)
  {
    for (local_14 = 0; local_14 < 0x10; local_14 = local_14 + 1) {
      if (*(int *)((&DAT_0001a0cc)[local_10] + local_14 * 0x100 + 0xd0) != 0) {
        local_18 = local_18 + 1;
      }
    }
  }
  printf("  Used Inodes: @d/@d\n",local_18,0x400);
  for (local_10 = 0; (local_10 < 0x100 && (*(int *)(&DAT_0001a1e4 + local_10 * 8) != 0));
      local_10 = local_10 + 1) {
    for (local_14 = 0; local_14 < 8; local_14 = local_14 + 1) {
      if (((int)(uint)(byte)(&free_list)[local_10 * 8] >> ((byte)local_14 & 0x1f) & 1U) != 0) {
        local_1c = local_1c + 1;
      }
    }
  }
  printf("  Used Blocks: @d/@d\n",local_1c,DAT_0001a0c8 >> 9);
  return 0;
}

// Function: ls
undefined4 ls(int param_1) {
  undefined4 uVar1;
  size_t sVar2;
  char *pcVar3;
  int local_20;
  char *local_1c;
  int local_18;
  int local_14;
  int local_10;
  
  if (param_1 == 0) {
    uVar1 = 0xffffffff;
  }
  else {
    local_10 = CheckFileExists(param_1,&local_20);
    if (local_10 == 0) {
      puts("No such directory");
      uVar1 = 0xffffffff;
    }
    else if (local_10 == 1) {
      puts("File is not a directory");
      uVar1 = 0xffffffff;
    }
    else if (local_10 == -1) {
      uVar1 = 0xffffffff;
    }
    else {
      local_18 = *(int *)(local_20 + 0xe0);
      if (local_18 == 0) {
        uVar1 = 0xffffffff;
      }
      else {
        printf("Directory listing of @s\n",param_1);
        for (local_10 = 0;
            (local_10 < 0x80 &&
            (local_1c = *(char **)(local_18 + local_10 * 4), local_1c != (char *)0x0));
            local_10 = local_10 + 1) {
          if (*(int *)(local_1c + 0xd0) == 2) {
            printf("d ",param_1);
          }
          else {
            printf("  ",param_1);
          }
          pcVar3 = local_1c;
          printf("@s",local_1c);
          sVar2 = strlen(local_1c);
          local_14 = 0x32 - sVar2;
          while (0 < local_14) {
            local_14 = local_14 + -1;
            printf(" ",pcVar3);
          }
          param_1 = *(int *)(local_1c + 0xd4);
          local_14 = local_14 + -1;
          printf(" @d\n",param_1);
        }
        uVar1 = 0;
      }
    }
  }
  return uVar1;
}

