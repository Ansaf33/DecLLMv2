// Function: cityMenu
undefined4 cityMenu(int param_1) {
  double *pdVar1;
  undefined4 uVar2;
  int iVar3;
  double dVar4;
  double *local_34;
  char local_2e [30];
  int local_10;
  
  local_10 = 0;
  local_34 = (double *)0x0;
  if (param_1 == 0) {
    uVar2 = 0;
  }
  else {
LAB_00011031:
    printf("\nCity: @s\n",param_1);
    printf("1) Display City Info\n");
    printf("2) Set Mayor\n");
    printf("3) Set Url\n");
    printf("4) Set Population\n");
    printf("5) Add Border\n");
    printf("6) Delete City and Exit Menu\n");
    printf("7) Exit Menu\n");
    printf("Selection: ");
    bzero(local_2e,0x1e);
    receive_until(local_2e,10,4);
    local_10 = atoi(local_2e);
    switch(local_10) {
    default:
      printf("Invalid\n");
      goto LAB_00011031;
    case 1:
      printCityInfo(param_1);
      goto LAB_00011031;
    case 2:
      printf("\n-> ");
      bzero(local_2e,0x1e);
      receive_until(local_2e,10,0x1d);
      local_10 = 0;
      while ((iVar3 = isalnum((int)local_2e[local_10]), iVar3 != 0 || (local_2e[local_10] == ' ')))
      {
        *(char *)(local_10 + param_1 + 0x14) = local_2e[local_10];
        local_10 = local_10 + 1;
      }
      *(undefined *)(local_10 + param_1 + 0x14) = 0;
      goto LAB_00011031;
    case 3:
      printf("\n-> ");
      bzero(local_2e,0x1e);
      receive_until(local_2e,10,0x1d);
      local_10 = 0;
      while ((((iVar3 = isalnum((int)local_2e[local_10]), iVar3 != 0 || (local_2e[local_10] == ':'))
              || (local_2e[local_10] == '.')) || (local_2e[local_10] == '/'))) {
        *(char *)(local_10 + param_1 + 0x32) = local_2e[local_10];
        local_10 = local_10 + 1;
      }
      *(undefined *)(local_10 + param_1 + 0x32) = 0;
      goto LAB_00011031;
    case 4:
      printf("\n-> ");
      bzero(local_2e,0x1e);
      receive_until(local_2e,10,0x1d);
      iVar3 = atoi(local_2e);
      *(int *)(param_1 + 0x50) = iVar3;
      goto LAB_00011031;
    case 5:
      if (*(int *)(param_1 + 0x54) < 0xb) {
        iVar3 = allocate(0x20,0,&local_34);
        if (iVar3 == 0) {
          printf("Lat Start: ");
          bzero(local_2e,0x1e);
          receive_until(local_2e,10,0x13);
          pdVar1 = local_34;
          dVar4 = atof(local_2e);
          *pdVar1 = dVar4;
          printf("Long Start: ");
          bzero(local_2e,0x1e);
          receive_until(local_2e,10,0x13);
          pdVar1 = local_34;
          dVar4 = atof(local_2e);
          pdVar1[1] = dVar4;
          printf("Lat End: ");
          bzero(local_2e,0x1e);
          receive_until(local_2e,10,0x13);
          pdVar1 = local_34;
          dVar4 = atof(local_2e);
          pdVar1[2] = dVar4;
          printf("Long End: ");
          bzero(local_2e,0x1e);
          receive_until(local_2e,10,0x13);
          pdVar1 = local_34;
          dVar4 = atof(local_2e);
          pdVar1[3] = dVar4;
          *(double **)(param_1 + 8 + (*(int *)(param_1 + 0x54) + 0x14) * 4) = local_34;
          *(int *)(param_1 + 0x54) = *(int *)(param_1 + 0x54) + 1;
        }
        else {
          local_34 = (double *)0x0;
        }
      }
      else {
        printf("Max Borders\n");
      }
      goto LAB_00011031;
    case 6:
      freeCity(param_1);
      uVar2 = 0;
      break;
    case 7:
      uVar2 = 1;
    }
  }
  return uVar2;
}

// Function: freeCharPtr
void freeCharPtr(char **param_1) {
  size_t sVar1;
  
  if ((param_1 != (char **)0x0) && (*param_1 != (char *)0x0)) {
    sVar1 = strlen(*param_1);
    deallocate(*param_1,sVar1 + 1);
    *param_1 = (char *)0x0;
  }
  return;
}

// Function: freeCity
void freeCity(int param_1) {
  int local_10;
  
  if (param_1 != 0) {
    for (local_10 = 0; local_10 < *(int *)(param_1 + 0x54); local_10 = local_10 + 1) {
      deallocate(param_1 + 0x58,0x20);
    }
    deallocate(param_1,0x80);
  }
  return;
}

// Function: initCity
undefined ** initCity(undefined **param_1) {
  undefined **ppuVar1;
  int local_8;
  
  ppuVar1 = &_GLOBAL_OFFSET_TABLE_;
  if (param_1 != (undefined **)0x0) {
    for (local_8 = 0; local_8 < 10; local_8 = local_8 + 1) {
      *(undefined *)(local_8 + (int)param_1) = 0;
      param_1[local_8 + 0x16] = (undefined *)0x0;
    }
    for (local_8 = 0; local_8 < 0x1e; local_8 = local_8 + 1) {
      *(undefined *)((int)param_1 + local_8 + 0x14) = 0;
      *(undefined *)((int)param_1 + local_8 + 0x32) = 0;
    }
    param_1[0x14] = (undefined *)0xffffffff;
    param_1[0x15] = (undefined *)0x0;
    ppuVar1 = param_1;
  }
  return ppuVar1;
}

// Function: printCityInfo
void printCityInfo(char *param_1) {
  undefined8 *puVar1;
  int local_10;
  
  if (param_1 != (char *)0x0) {
    printf("\t\t\t\tCity: ");
    if (*param_1 == '\0') {
      printf("Unknown\n");
    }
    else {
      printf("@s\n",param_1);
    }
    if (-1 < *(int *)(param_1 + 0x50)) {
      printf("\t\t\t\t\tPopulation: @d\n",*(undefined4 *)(param_1 + 0x50));
    }
    if (param_1[0x14] != '\0') {
      printf("\t\t\t\t\tMayor: @s\n",param_1 + 0x14);
    }
    if (param_1[0x32] != '\0') {
      printf("\t\t\t\t\tUrl: @s\n",param_1 + 0x32);
    }
    for (local_10 = 0; local_10 < *(int *)(param_1 + 0x54); local_10 = local_10 + 1) {
      puVar1 = *(undefined8 **)(param_1 + (local_10 + 0x14) * 4 + 8);
      if (puVar1 != (undefined8 *)0x0) {
        printf("\t\t\t\t\tBorder: @f @f @f @f\n",*puVar1,puVar1[1],puVar1[2],puVar1[3]);
      }
    }
  }
  return;
}

// Function: skipOpen
undefined4 skipOpen(int param_1,int param_2) {
  undefined4 uVar1;
  int iVar2;
  
  if (param_1 == 0) {
    uVar1 = 0;
  }
  else {
    skipWhiteSpace(param_1);
    iVar2 = atChar(param_1,0x7b);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      iVar2 = incChar(param_1);
      if (iVar2 == -1) {
        uVar1 = 0;
      }
      else {
        skipWhiteSpace(param_1);
        if (param_2 != 0) {
          iVar2 = atChar(param_1,0x23);
          if (iVar2 == 0) {
            return 0;
          }
          iVar2 = incChar(param_1);
          if (iVar2 == -1) {
            return 0;
          }
        }
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}

// Function: cityTopLevel
char * cityTopLevel(int *param_1) {
  char *pcVar1;
  int iVar2;
  size_t sVar3;
  undefined4 uVar4;
  char *local_28;
  int local_24;
  char *local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  int local_10;
  
  local_10 = 0;
  local_20 = (char *)0x0;
  local_24 = 0;
  local_14 = 0;
  local_28 = (char *)0x0;
  local_18 = 0;
  if ((param_1 != (int *)0x0) && (iVar2 = skipOpen(param_1,0), iVar2 != 0)) {
    getIndex(param_1,&local_24);
    local_14 = skipAlpha(param_1);
    if ((local_14 != -1) &&
       ((local_14 != local_24 &&
        (local_28 = (char *)copyData(param_1,local_24,local_14), local_28 != (char *)0x0)))) {
      iVar2 = strcmp(local_28,"City");
      if (iVar2 == 0) {
        sVar3 = strlen(local_28);
        deallocate(local_28,sVar3 + 1);
        skipWhiteSpace(param_1);
        iVar2 = atChar(param_1,0x7d);
        if (iVar2 != 0) {
          incChar(param_1);
          skipWhiteSpace(param_1);
          local_10 = param_1[2];
          iVar2 = allocate(0x80,0,&local_20);
          if (iVar2 == 0) {
            initCity(local_20);
            local_28 = (char *)pullNextElementName(param_1);
            while (local_28 != (char *)0x0) {
              local_1c = elementNameToEnum(local_28);
              sVar3 = strlen(local_28);
              deallocate(local_28,sVar3 + 1);
              pcVar1 = local_20;
              switch(local_1c) {
              default:
                printf("!!Invalid element ID for City\n");
                goto LAB_00011d16;
              case 1:
                local_28 = (char *)extractName(param_1);
                if (local_28 == (char *)0x0) {
                  local_28 = (char *)0x0;
                  goto LAB_00011d16;
                }
                bzero(local_20,0x14);
                strncpy(local_20,local_28,0x13);
                sVar3 = strlen(local_28);
                deallocate(local_28,sVar3 + 1);
                local_28 = (char *)0x0;
                break;
              case 10:
                uVar4 = extractPopulation(param_1);
                *(undefined4 *)(pcVar1 + 0x50) = uVar4;
                if (*(int *)(local_20 + 0x50) < 0) goto LAB_00011d16;
                break;
              case 0xe:
                if (9 < *(int *)(local_20 + 0x54)) goto LAB_00011d16;
                local_18 = *(int *)(local_20 + 0x54);
                uVar4 = extractBorder(param_1);
                *(undefined4 *)(pcVar1 + (local_18 + 0x14) * 4 + 8) = uVar4;
                if (*(int *)(local_20 + (local_18 + 0x14) * 4 + 8) == 0) goto LAB_00011d16;
                *(int *)(local_20 + 0x54) = *(int *)(local_20 + 0x54) + 1;
                break;
              case 0x17:
                local_28 = (char *)extractMayor(param_1);
                if (local_28 == (char *)0x0) {
                  local_28 = (char *)0x0;
                  goto LAB_00011d16;
                }
                bzero(local_20 + 0x14,0x1e);
                strcpy(local_20 + 0x14,local_28);
                freeCharPtr(&local_28);
                break;
              case 0x18:
                local_28 = (char *)extractUrl(param_1);
                if (local_28 == (char *)0x0) {
                  local_28 = (char *)0x0;
                  goto LAB_00011d16;
                }
                bzero(local_20 + 0x32,0x1e);
                strncpy(local_20 + 0x32,local_28,0x1d);
                freeCharPtr(&local_28);
              }
              local_10 = param_1[2];
              local_28 = (char *)pullNextElementName(param_1);
            }
            iVar2 = skipOpen(param_1,1);
            if (iVar2 != 0) {
              getIndex(param_1,&local_24);
              local_14 = skipAlpha(param_1);
              if (((local_14 != -1) && (local_14 != local_24)) &&
                 (local_28 = (char *)copyData(param_1,local_24,local_14), local_28 != (char *)0x0))
              {
                iVar2 = strcmp(local_28,"City");
                if (iVar2 == 0) {
                  sVar3 = strlen(local_28);
                  deallocate(local_28,sVar3 + 1);
                  skipWhiteSpace(param_1);
                  iVar2 = atChar(param_1,0x7d);
                  if (iVar2 != 0) {
                    incChar(param_1);
                    return local_20;
                  }
                }
                else {
                  sVar3 = strlen(local_28);
                  deallocate(local_28,sVar3 + 1);
                }
              }
            }
          }
          else {
            local_20 = (char *)0x0;
          }
LAB_00011d16:
          if (local_20 != (char *)0x0) {
            freeCity(local_20);
            local_20 = (char *)0x0;
          }
          printf("!!Error at: @s\n",local_10 + *param_1);
          param_1[2] = local_10;
        }
      }
      else {
        sVar3 = strlen(local_28);
        deallocate(local_28,sVar3 + 1);
      }
    }
  }
  return local_20;
}

// Function: extractMayor
char * extractMayor(int *param_1) {
  int iVar1;
  int iVar2;
  int iVar3;
  char *local_24;
  char *local_20 [4];
  
  local_20[0] = (char *)0x0;
  local_24 = (char *)0x0;
  if (param_1 == (int *)0x0) {
    local_20[0] = (char *)0x0;
  }
  else {
    iVar1 = skipOpen(param_1,0);
    if (iVar1 == 0) {
      local_20[0] = (char *)0x0;
    }
    else {
      iVar1 = param_1[2];
      iVar2 = skipAlpha(param_1);
      if ((iVar2 == -1) || (iVar1 == iVar2)) {
        local_20[0] = (char *)0x0;
      }
      else {
        local_20[0] = (char *)copyData(param_1,iVar1,iVar2);
        if (local_20[0] == (char *)0x0) {
          local_20[0] = (char *)0x0;
        }
        else {
          iVar1 = strcmp(local_20[0],"Mayor");
          if (iVar1 == 0) {
            freeCharPtr(local_20);
            skipWhiteSpace(param_1);
            iVar1 = atChar(param_1,0x7d);
            if (iVar1 == 0) {
              local_20[0] = (char *)0x0;
            }
            else {
              incChar(param_1);
              iVar1 = skipWhiteSpace(param_1);
              iVar2 = skipToNonAlphaNumSpace(param_1);
              if ((iVar2 < 0) || (iVar1 == iVar2)) {
                local_20[0] = (char *)0x0;
              }
              else {
                while (iVar3 = isspace((int)*(char *)(*param_1 + iVar2 + -1)), iVar3 != 0) {
                  iVar2 = iVar2 + -1;
                }
                local_20[0] = (char *)copyData(param_1,iVar1,iVar2);
                if (local_20[0] == (char *)0x0) {
                  local_20[0] = (char *)0x0;
                }
                else {
                  skipWhiteSpace(param_1);
                  iVar1 = skipOpen(param_1,1);
                  if (iVar1 != 0) {
                    iVar1 = param_1[2];
                    iVar2 = skipAlpha(param_1);
                    if (((iVar2 != -1) && (iVar1 != iVar2)) &&
                       (local_24 = (char *)copyData(param_1,iVar1,iVar2), local_24 != (char *)0x0))
                    {
                      iVar1 = strcmp(local_24,"Mayor");
                      if (iVar1 == 0) {
                        skipWhiteSpace(param_1);
                        iVar1 = atChar(param_1,0x7d);
                        if (iVar1 != 0) {
                          incChar(param_1);
                          return local_20[0];
                        }
                      }
                      else {
                        freeCharPtr(&local_24);
                      }
                    }
                  }
                  freeCharPtr(local_20);
                }
              }
            }
          }
          else {
            freeCharPtr(local_20);
            local_20[0] = (char *)0x0;
          }
        }
      }
    }
  }
  return local_20[0];
}

// Function: extractUrl
char * extractUrl(int param_1) {
  int iVar1;
  char *local_18;
  undefined4 local_14;
  char *local_10 [2];
  
  local_10[0] = (char *)0x0;
  local_14 = 0;
  local_18 = (char *)0x0;
  if ((param_1 != 0) && (iVar1 = skipOpen(param_1,0), iVar1 != 0)) {
    getIndex(param_1,&local_14);
    skipAlpha(param_1);
    local_10[0] = (char *)copyData(param_1,local_14,*(undefined4 *)(param_1 + 8));
    if (local_10[0] != (char *)0x0) {
      iVar1 = strcmp(local_10[0],"Url");
      if (iVar1 == 0) {
        skipWhiteSpace(param_1);
        iVar1 = atChar(param_1,0x7d);
        if (iVar1 != 0) {
          incChar(param_1);
          skipWhiteSpace(param_1);
          getIndex(param_1,&local_14);
          skipUrl(param_1);
          local_10[0] = (char *)copyData(param_1,local_14,*(undefined4 *)(param_1 + 8));
          if (local_10[0] != (char *)0x0) {
            skipWhiteSpace(param_1);
            iVar1 = skipOpen(param_1,1);
            if (iVar1 != 0) {
              getIndex(param_1,&local_14);
              skipAlpha(param_1);
              local_18 = (char *)copyData(param_1,local_14,*(undefined4 *)(param_1 + 8));
              if (local_18 != (char *)0x0) {
                iVar1 = strcmp(local_18,"Url");
                if (iVar1 == 0) {
                  freeCharPtr(&local_18);
                  skipWhiteSpace(param_1);
                  iVar1 = atChar(param_1,0x7d);
                  if (iVar1 != 0) {
                    incChar(param_1);
                    return local_10[0];
                  }
                }
                else {
                  freeCharPtr(&local_18);
                }
              }
            }
            if (local_10[0] != (char *)0x0) {
              freeCharPtr(local_10);
            }
          }
        }
      }
      else {
        freeCharPtr(local_10);
      }
    }
  }
  return local_10[0];
}

