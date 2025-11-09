// Function: InitFilesystem
void InitFilesystem(void) {
  undefined uVar1;
  size_t sVar2;
  undefined local_54 [31];
  undefined auStack_35 [11];
  undefined auStack_2a [22];
  char *local_14;
  byte local_d;
  
  local_14 = &DAT_4347c000;
  bzero(FS,0x10640);
  bzero(FH,0xc0);
  bzero(auStack_35 + 1,0x20);
  for (local_d = 0; local_d < 10; local_d = local_d + 1) {
    uVar1 = random_in_range(0x41,0x7a);
    auStack_35[local_d + 1] = uVar1;
  }
  auStack_35[local_d + 1] = 0;
  bzero(local_54,0x20);
  for (local_d = 0; local_d < 0x1f; local_d = local_d + 1) {
    uVar1 = random_in_range(0x41,0x7a);
    local_54[local_d] = uVar1;
  }
  local_54[local_d] = 0;
  strcpy(FS,"passwd");
  strcpy(FS + 0x1020,"root");
  strcpy(FS + 0x1040,"root");
  FS._4192_2_ = 0x700;
  sprintf(FS + 0x20,"$s:secure:secure\ncrs:crs:crs\n$s:root:root",local_54,auStack_35 + 1);
  sVar2 = strlen(FS + 0x20);
  FS._4194_2_ = (undefined2)sVar2;
  strcpy(FS + 0x1064,"confidential");
  strcpy(FS + 0x2084,"secure");
  strcpy(FS + 0x20a4,"secure");
  FS._8388_2_ = 0x700;
  local_14 = local_14 + 4;
  local_d = 0;
  while (local_d < 4) {
    if ((*local_14 == '\0') || (*local_14 == '\n')) {
      local_d = 0;
      local_14 = local_14 + 1;
    }
    else {
      FS[local_d + 0x1084] = *local_14;
      local_14 = local_14 + 1;
      local_d = local_d + 1;
    }
  }
  FS._8390_2_ = 4;
  return;
}

// Function: ListFiles
void ListFiles(void) {
  byte local_1d;
  
  puts("Directory listing");
  printf("$-32s $-32s $-32s $5s\n\r","Filename","Owner","Group",&DAT_0001705c);
  for (local_1d = 0; local_1d < 0x10; local_1d = local_1d + 1) {
    if (FS[(uint)local_1d * 0x1064] != '\0') {
      printf("$-32s $-32s $-32s $5d\n\r",FS + (uint)local_1d * 0x1064,
             (uint)local_1d * 0x1064 + 0x1b240,(uint)local_1d * 0x1064 + 0x1b260,
             (uint)*(ushort *)(FS + (uint)local_1d * 0x1064 + 0x1062));
    }
  }
  return;
}

// Function: Mode2Perms
ushort Mode2Perms(char *param_1) {
  ushort local_6;
  
  local_6 = 0;
  if (*param_1 == 'r') {
    local_6 = 4;
  }
  if (*param_1 == 'w') {
    local_6 = local_6 | 2;
  }
  return local_6;
}

// Function: fopen
FILE * fopen(char *__filename,char *__modes) {
  ushort uVar1;
  ushort uVar2;
  FILE *pFVar3;
  size_t sVar4;
  int iVar5;
  char in_stack_0000000c;
  byte local_15;
  char *local_14;
  byte local_d;
  
  local_14 = (char *)0x0;
  if ((__filename == (char *)0x0) || (__modes == (char *)0x0)) {
    pFVar3 = (FILE *)0x0;
  }
  else {
    sVar4 = strlen(__filename);
    if (sVar4 < 0x20) {
      sVar4 = strlen(__modes);
      if (sVar4 < 2) {
        if ((*__modes == 'r') || (*__modes == 'w')) {
          for (local_15 = 0; local_15 < 0x10; local_15 = local_15 + 1) {
            if ((FS[(uint)local_15 * 0x1064] == '\0') && (local_14 == (char *)0x0)) {
              local_14 = FS + (uint)local_15 * 0x1064;
            }
            iVar5 = strcmp(__filename,FS + (uint)local_15 * 0x1064);
            if (iVar5 == 0) break;
          }
          if (local_15 == 0x10) {
            if (*__modes == 'r') {
              return (FILE *)0x0;
            }
          }
          else if ((in_stack_0000000c == '\0') && (iVar5 = strcmp(&DAT_0002b921,"root"), iVar5 != 0)
                  ) {
            iVar5 = strcmp(FS + (uint)local_15 * 0x1064 + 0x1020,&DAT_0002b921);
            if (iVar5 == 0) {
              uVar1 = *(ushort *)(FS + (uint)local_15 * 0x1064 + 0x1060);
              uVar2 = Mode2Perms(__modes);
              if ((uVar2 & uVar1 >> 8 & 0xf) == 0) {
                return (FILE *)0x0;
              }
            }
            else {
              iVar5 = strcmp(FS + (uint)local_15 * 0x1064 + 0x1040,&DAT_0002b941);
              if (iVar5 == 0) {
                uVar1 = *(ushort *)(FS + (uint)local_15 * 0x1064 + 0x1060);
                uVar2 = Mode2Perms(__modes);
                if ((uVar2 & (ushort)((int)(uint)uVar1 >> 4) & 0xf) == 0) {
                  return (FILE *)0x0;
                }
              }
              else {
                uVar1 = *(ushort *)(FS + (uint)local_15 * 0x1064 + 0x1060);
                uVar2 = Mode2Perms(__modes);
                if ((uVar2 & uVar1 & 0xf) == 0) {
                  return (FILE *)0x0;
                }
              }
            }
          }
          local_d = 0;
          while ((local_d < 0x10 && (*(int *)(FH + (uint)local_d * 0xc) != 0))) {
            local_d = local_d + 1;
          }
          if (local_d == 0x10) {
            pFVar3 = (FILE *)0x0;
          }
          else {
            if (*__modes == 'w') {
              if (local_15 == 0x10) {
                if (local_14 == (char *)0x0) {
                  return (FILE *)0x0;
                }
                strcpy(local_14,__filename);
                strcpy(local_14 + 0x1020,&DAT_0002b921);
                strcpy(local_14 + 0x1040,&DAT_0002b941);
                *(undefined2 *)(local_14 + 0x1060) = 0x700;
                bzero(local_14 + 0x20,0x1000);
                *(undefined2 *)(local_14 + 0x1062) = 0;
                *(char **)(FH + (uint)local_d * 0xc) = local_14;
                FH[(uint)local_d * 0xc + 8] = 2;
              }
              else {
                *(undefined1 **)(FH + (uint)local_d * 0xc) = FS + (uint)local_15 * 0x1064;
                FH[(uint)local_d * 0xc + 8] = 2;
                bzero((void *)(*(int *)(FH + (uint)local_d * 0xc) + 0x20),0x1000);
                *(undefined2 *)(*(int *)(FH + (uint)local_d * 0xc) + 0x1062) = 0;
              }
            }
            else {
              *(undefined1 **)(FH + (uint)local_d * 0xc) = FS + (uint)local_15 * 0x1064;
              FH[(uint)local_d * 0xc + 8] = 4;
            }
            *(int *)(FH + (uint)local_d * 0xc + 4) = *(int *)(FH + (uint)local_d * 0xc) + 0x20;
            pFVar3 = (FILE *)(FH + (uint)local_d * 0xc);
          }
        }
        else {
          pFVar3 = (FILE *)0x0;
        }
      }
      else {
        pFVar3 = (FILE *)0x0;
      }
    }
    else {
      pFVar3 = (FILE *)0x0;
    }
  }
  return pFVar3;
}

// Function: fgets
char * fgets(char *__s,int __n,FILE *__stream) {
  char *pcVar1;
  int local_8;
  
  if (((__s == (char *)0x0) || (__stream == (FILE *)0x0)) || (__n == 0)) {
    __s = (char *)0x0;
  }
  else if (__stream->_IO_read_ptr == (char *)0x0) {
    __s = (char *)0x0;
  }
  else if (*__stream->_IO_read_ptr == '\0') {
    __s = (char *)0x0;
  }
  else {
    local_8 = 0;
    for (; (*__stream->_IO_read_ptr != '\0' && (__n != 1)); __n = __n + -1) {
      if (*__stream->_IO_read_ptr == '\n') {
        __stream->_IO_read_ptr = __stream->_IO_read_ptr + 1;
        __s[local_8] = '\0';
        return __s;
      }
      pcVar1 = __stream->_IO_read_ptr;
      __stream->_IO_read_ptr = pcVar1 + 1;
      __s[local_8] = *pcVar1;
      local_8 = local_8 + 1;
    }
    __s[local_8] = '\0';
  }
  return __s;
}

// Function: fclose
int fclose(FILE *__stream) {
  if (__stream != (FILE *)0x0) {
    __stream->_flags = 0;
    __stream->_IO_read_ptr = (char *)0x0;
  }
  return (uint)(__stream != (FILE *)0x0);
}

// Function: fread
size_t fread(void *__ptr,size_t __size,size_t __n,FILE *__stream) {
  size_t sVar1;
  
  if ((((__ptr == (void *)0x0) || (__stream == (FILE *)0x0)) || (__size == 0)) || (__n == 0)) {
    sVar1 = 0;
  }
  else if (((__stream->_flags == 0) || (__stream->_IO_read_ptr == (char *)0x0)) ||
          (0x1000 < __size * __n)) {
    sVar1 = 0;
  }
  else if ((uint)*(ushort *)(__stream->_flags + 0x1062) < __size * __n) {
    memcpy(__ptr,(void *)(__stream->_flags + 0x20),(uint)*(ushort *)(__stream->_flags + 0x1062));
    sVar1 = (size_t)*(ushort *)(__stream->_flags + 0x1062);
  }
  else {
    memcpy(__ptr,(void *)(__stream->_flags + 0x20),__size * __n);
    sVar1 = __size * __n;
  }
  return sVar1;
}

// Function: fwrite
size_t fwrite(void *__ptr,size_t __size,size_t __n,FILE *__s) {
  size_t sVar1;
  
  if ((__ptr == (void *)0x0) || (__s == (FILE *)0x0)) {
    sVar1 = 0;
  }
  else if ((__s->_flags == 0) ||
          (((__s->_IO_read_ptr == (char *)0x0 || (0x1000 < __size * __n)) ||
           (*(char *)&__s->_IO_read_end != '\x02')))) {
    sVar1 = 0;
  }
  else {
    memcpy(__s->_IO_read_ptr,__ptr,__size * __n);
    *(short *)(__s->_flags + 0x1062) = (short)__size * (short)__n + *(short *)(__s->_flags + 0x1062)
    ;
    sVar1 = __size * __n;
  }
  return sVar1;
}

// Function: Dump
undefined4 Dump(char *param_1) {
  undefined4 uVar1;
  char *pcVar2;
  char local_410 [1024];
  FILE *local_10;
  
  if (param_1 == (char *)0x0) {
    uVar1 = 0;
  }
  else {
    uVar1 = 0;
    local_10 = fopen(param_1,"r");
    if (local_10 == (FILE *)0x0) {
      printf("Unable to open file \'$s\'\n\r",param_1,uVar1);
      uVar1 = 0;
    }
    else {
      while( true ) {
        pcVar2 = fgets(local_410,0x400,local_10);
        if (pcVar2 == (char *)0x0) break;
        puts(local_410);
      }
      fclose(local_10);
      uVar1 = 1;
    }
  }
  return uVar1;
}

