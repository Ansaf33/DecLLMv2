// Function: RandomInfluent
void RandomInfluent(void) {
  uint uVar1;
  
  uVar1 = random_in_range(9,0x1e);
  DAT_0001edc0 = (double)(ulonglong)uVar1 / DAT_000192c8;
  uVar1 = random_in_range(0x76c,4000);
  DAT_0001edc8 = (double)(ulonglong)uVar1;
  uVar1 = random_in_range(2000,5000);
  DAT_0001edd0 = (double)(ulonglong)uVar1;
  uVar1 = random_in_range(0x1ea,0x2ee);
  DAT_0001edd8 = (double)(ulonglong)uVar1;
  uVar1 = random_in_range(0x4b,0x96);
  DAT_0001ede0 = (double)(ulonglong)uVar1;
  return;
}

// Function: InitInfluent
void InitInfluent(void) {
  bzero(&Influent,0xe4);
  Influent = 0;
  strncpy(&DAT_0001ed41,"Influent",0x1f);
  DAT_0001edc0 = DAT_000192d0;
  DAT_0001edc8 = DAT_000192d8;
  DAT_0001edd0 = DAT_000192e0;
  DAT_0001edd8 = DAT_000192e8;
  DAT_0001ede0 = DAT_000192f0;
  DAT_0001edac = &Headworks;
  return;
}

// Function: GetValvePctFlow
undefined ** GetValvePctFlow(undefined **param_1,byte param_2) {
  undefined **ppuVar1;
  double local_14;
  byte local_5;
  
  ppuVar1 = &_GLOBAL_OFFSET_TABLE_;
  if ((((param_1 != (undefined **)0x0) && (param_2 < 6)) && (*(char *)param_1 == '\x05')) &&
     (param_1[param_2 + 0x1b] != (undefined *)0x0)) {
    local_14 = 0.0;
    for (local_5 = 0; local_5 < 5; local_5 = local_5 + 1) {
      if (param_1[local_5 + 0x1b] != (undefined *)0x0) {
        local_14 = local_14 + *(double *)(param_1 + (local_5 + 0x16) * 2 + 3);
      }
    }
    ppuVar1 = param_1;
    if (local_14 == 0.0) {
      ppuVar1 = &_GLOBAL_OFFSET_TABLE_;
    }
  }
  return ppuVar1;
}

// Function: SetValveOutput
void SetValveOutput(char *param_1,byte param_2,undefined4 param_3,undefined4 param_4) {
  if ((((param_1 != (char *)0x0) && (param_2 < 6)) &&
      ((double)CONCAT44(param_4,param_3) <= DAT_000192f8)) &&
     ((*param_1 == '\x05' && (*(int *)(param_1 + (param_2 + 0x18) * 4 + 0xc) != 0)))) {
    *(ulonglong *)(param_1 + (param_2 + 0x16) * 8 + 0xc) = CONCAT44(param_4,param_3);
  }
  return;
}

// Function: SetSystemName
undefined ** SetSystemName(int param_1,char *param_2) {
  undefined **ppuVar1;
  
  ppuVar1 = &_GLOBAL_OFFSET_TABLE_;
  if (param_1 != 0) {
    ppuVar1 = (undefined **)strncpy((char *)(param_1 + 1),param_2,0x1f);
  }
  return ppuVar1;
}

// Function: ChangeAerationPercent
void ChangeAerationPercent(int param_1,undefined4 param_2,undefined4 param_3) {
  if (((Asp == '\x02') && (0.0 <= (double)CONCAT44(param_3,param_2))) &&
     ((double)CONCAT44(param_3,param_2) <= DAT_000192f8)) {
    *(ulonglong *)(param_1 + 0x2c) = CONCAT44(param_3,param_2);
  }
  return;
}

// Function: ChangeGlycerinRate
void ChangeGlycerinRate(int param_1,undefined4 param_2,undefined4 param_3) {
  if (((Asp == '\x02') && (0.0 <= (double)CONCAT44(param_3,param_2))) &&
     ((double)CONCAT44(param_3,param_2) <= DAT_000192f8)) {
    *(ulonglong *)(param_1 + 0x3c) = CONCAT44(param_3,param_2);
  }
  return;
}

// Function: ChangeAlumRate
void ChangeAlumRate(int param_1,undefined4 param_2,undefined4 param_3) {
  if (((Asp == '\x02') && (0.0 <= (double)CONCAT44(param_3,param_2))) &&
     ((double)CONCAT44(param_3,param_2) <= DAT_000192f8)) {
    *(ulonglong *)(param_1 + 0x44) = CONCAT44(param_3,param_2);
  }
  return;
}

// Function: ChangeChlorineRate
void ChangeChlorineRate(char *param_1,undefined4 param_2,undefined4 param_3) {
  if ((((param_1 != (char *)0x0) && (*param_1 == '\x04')) &&
      (0.0 <= (double)CONCAT44(param_3,param_2))) &&
     ((double)CONCAT44(param_3,param_2) <= DAT_000192f8)) {
    *(ulonglong *)(param_1 + 0x58) = CONCAT44(param_3,param_2);
  }
  return;
}

// Function: ChangeNumActiveFilters
void ChangeNumActiveFilters(char *param_1,byte param_2) {
  if (((param_1 != (char *)0x0) && (*param_1 == '\x03')) && (param_2 <= (byte)param_1[0x55])) {
    param_1[0x54] = param_2;
  }
  return;
}

// Function: InitHeadworks
void InitHeadworks(void) {
  bzero(&Headworks,0xe4);
  Headworks = 1;
  strncpy(&DAT_0001e241,"Headworks",0x1f);
  DAT_0001e2e8 = &Influent;
  DAT_0001e2ac = &HeadworksOutputValve;
  DAT_0001e264 = DAT_00019300;
  return;
}

// Function: InitHeadworksOutputValve
void InitHeadworksOutputValve(void) {
  byte local_d;
  
  bzero(&HeadworksOutputValve,0xe4);
  HeadworksOutputValve = 5;
  strncpy(&DAT_0001ee41,"V1",0x1f);
  DAT_0001eee8 = &Headworks;
  for (local_d = 0; local_d < NumAsp; local_d = local_d + 1) {
    *(undefined1 **)(&DAT_0001ee4c + (local_d + 0x18) * 4) = &Asp + (uint)local_d * 0xe4;
    *(double *)(&DAT_0001ee4c + (local_d + 0x16) * 8) = DAT_000192f8 / (double)(uint)NumAsp;
  }
  return;
}

// Function: InitAsp
void InitAsp(void) {
  byte local_d;
  
  bzero(&Asp,0x474);
  for (local_d = 0; local_d < NumAsp; local_d = local_d + 1) {
    (&Asp)[(uint)local_d * 0xe4] = 2;
    cromu_sprintf(&DAT_0001ef41 + (uint)local_d * 0xe4,"Asp$d",local_d);
    *(undefined1 **)(&DAT_0001efe8 + (uint)local_d * 0xe4) = &HeadworksOutputValve;
    *(undefined1 **)(&DAT_0001efac + (uint)local_d * 0xe4) = &AspOutputValve;
    *(undefined8 *)(&DAT_0001ef6c + (uint)local_d * 0xe4) = DAT_00019308;
    *(undefined8 *)(&DAT_0001ef74 + (uint)local_d * 0xe4) = DAT_00019310;
    *(undefined8 *)(&DAT_0001ef7c + (uint)local_d * 0xe4) = DAT_00019318;
    *(undefined8 *)(&DAT_0001ef84 + (uint)local_d * 0xe4) = DAT_00019320;
  }
  return;
}

// Function: InitAspOutputValve
void InitAspOutputValve(void) {
  byte local_d;
  
  bzero(&AspOutputValve,0xe4);
  AspOutputValve = 5;
  strncpy(&DAT_0001e041,"V2",0x1f);
  for (local_d = 0; local_d < NumAsp; local_d = local_d + 1) {
    *(undefined1 **)(&DAT_0001e048 + (local_d + 0x28) * 4) = &Asp + (uint)local_d * 0xe4;
  }
  for (local_d = 0; local_d < NumFilters; local_d = local_d + 1) {
    *(undefined1 **)(&DAT_0001e04c + (local_d + 0x18) * 4) = Filter + (uint)local_d * 0xe4;
    *(double *)(&DAT_0001e04c + (local_d + 0x16) * 8) = DAT_000192f8 / (double)(uint)NumFilters;
  }
  return;
}

// Function: InitFilters
void InitFilters(void) {
  byte local_d;
  
  bzero(Filter,0x474);
  for (local_d = 0; local_d < NumFilters; local_d = local_d + 1) {
    Filter[(uint)local_d * 0xe4] = 3;
    cromu_sprintf((uint)local_d * 0xe4 + 0x1e341,"Filter$d",local_d);
    *(undefined1 **)(Filter + (uint)local_d * 0xe4 + 0xa8) = &AspOutputValve;
    *(undefined1 **)(Filter + (uint)local_d * 0xe4 + 0x6c) = &FilterOutputValve;
  }
  Filter[85] = 2;
  Filter[84] = 1;
  Filter[313] = 4;
  Filter[312] = 2;
  Filter[541] = 4;
  Filter[540] = 2;
  return;
}

// Function: InitFilterOutputValve
void InitFilterOutputValve(void) {
  byte local_d;
  
  bzero(&FilterOutputValve,0xe4);
  FilterOutputValve = 5;
  strncpy(&DAT_0001e7c1,"V3",0x1f);
  for (local_d = 0; local_d < NumFilters; local_d = local_d + 1) {
    *(undefined1 **)(&DAT_0001e7c8 + (local_d + 0x28) * 4) = Filter + (uint)local_d * 0xe4;
  }
  for (local_d = 0; local_d < NumDisinfection; local_d = local_d + 1) {
    *(undefined1 **)(&DAT_0001e7cc + (local_d + 0x18) * 4) = Disinfection + (uint)local_d * 0xe4;
    *(double *)(&DAT_0001e7cc + (local_d + 0x16) * 8) = DAT_000192f8 / (double)(uint)NumDisinfection
    ;
  }
  return;
}

// Function: InitDisinfection
void InitDisinfection(void) {
  byte local_d;
  
  bzero(Disinfection,0x474);
  for (local_d = 0; local_d < NumDisinfection; local_d = local_d + 1) {
    Disinfection[(uint)local_d * 0xe4] = 4;
    cromu_sprintf((uint)local_d * 0xe4 + 0x1e8c1,"Disinfection$d",local_d);
    *(undefined1 **)(Disinfection + (uint)local_d * 0xe4 + 0xa8) = &FilterOutputValve;
    *(undefined1 **)(Disinfection + (uint)local_d * 0xe4 + 0x6c) = &DisinfectionOutputValve;
    *(undefined8 *)(Disinfection + (uint)local_d * 0xe4 + 0x58) = 0x3ff0000000000000;
  }
  return;
}

// Function: InitDisinfectionOutputValve
void InitDisinfectionOutputValve(void) {
  byte local_d;
  
  bzero(&DisinfectionOutputValve,0xe4);
  DisinfectionOutputValve = 5;
  strncpy(&DAT_0001e141,"V4",0x1f);
  for (local_d = 0; local_d < NumDisinfection; local_d = local_d + 1) {
    *(undefined1 **)(&DAT_0001e148 + (local_d + 0x28) * 4) = Disinfection + (uint)local_d * 0xe4;
  }
  DAT_0001e1ac = &Effluent;
  DAT_0001e1fc = DAT_000192f8;
  return;
}

// Function: InitEffluent
void InitEffluent(void) {
  bzero(&Effluent,0xe4);
  Effluent = 6;
  strncpy(&DAT_0001f3c1,"Effluent",0x1f);
  DAT_0001f468 = &DisinfectionOutputValve;
  return;
}

// Function: CheckEffluentQuality
void CheckEffluentQuality(void) {
  if ((((DAT_00019330 < DAT_0001f448 / (DAT_00019328 * DAT_0001f440)) ||
       (DAT_00019330 < DAT_0001f450 / (DAT_00019328 * DAT_0001f440))) ||
      (_DAT_00019338 < DAT_0001f458 / (DAT_00019328 * DAT_0001f440))) ||
     ((1.0 < DAT_0001f460 / (DAT_00019328 * DAT_0001f440) || (DAT_0001f416 == '\0')))) {
    DAT_0001f428 = DAT_0001f428 + '\x01';
  }
  return;
}

// Function: PrintPlantSchematic
void PrintPlantSchematic(void) {
  cromu_puts("Plant Schematic");
  cromu_puts(
            "#####################################################################################################################################"
            );
  cromu_puts(
            "                                         --------                -----------               -----------------                         "
            );
  cromu_puts(
            "                                -------> | ASP1 | ------  -----> | Filter1 | -----  -----> | Disinfection1 | -----                   "
            );
  cromu_puts(
            "                                |        --------      |  |      -----------     |  |      -----------------     |                   "
            );
  cromu_puts(
            "                                |                      |  |                      |  |                            |                   "
            );
  cromu_puts(
            "                                |                      |  |                      |  |                            |                   "
            );
  cromu_puts(
            "                                0                      v  0                      v  0                            v                   "
            );
  cromu_puts(
            "            -------------     ------     --------     ------     -----------    ------     -----------------    ------               "
            );
  cromu_puts(
            "Influent -> | Headworks | --> | V1 | 1-> | ASP2 | --> | V2 | 1-> | Filter2 | -> | V3 | 1-> | Disinfection2 | -> | V4 | 0-> Effluent  "
            );
  cromu_puts(
            "            -------------     ------     --------     ------     -----------    ------     -----------------    ------               "
            );
  cromu_puts(
            "                                2                      ^  2                      ^  2                            ^                   "
            );
  cromu_puts(
            "                                |                      |  |                      |  |                            |                   "
            );
  cromu_puts(
            "                                |                      |  |                      |  |                            |                   "
            );
  cromu_puts(
            "                                |        --------      |  |      -----------     |  |      -----------------     |                   "
            );
  cromu_puts(
            "                                -------> | ASP3 | ------  -----> | Filter3 | -----  -----> | Disinfection3 | -----                   "
            );
  cromu_puts(
            "                                         --------                -----------               -----------------                         "
            );
  cromu_puts(
            "#####################################################################################################################################"
            );
  return;
}

// Function: PrintPlantStatus
void PrintPlantStatus(void) {
  undefined4 uVar1;
  
  uVar1 = PrintPlantStatusBuf();
  cromu_puts(uVar1);
  return;
}

// Function: PrintPlantStatusBuf
undefined1 * PrintPlantStatusBuf(void) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  
  iVar1 = cromu_sprintf();
  iVar2 = cromu_sprintf();
  iVar3 = cromu_sprintf();
  iVar4 = cromu_sprintf();
  iVar5 = cromu_sprintf();
  iVar6 = cromu_sprintf();
  iVar7 = cromu_sprintf();
  iVar8 = cromu_sprintf();
  iVar9 = cromu_sprintf();
  iVar10 = cromu_sprintf();
  iVar11 = cromu_sprintf();
  iVar12 = cromu_sprintf();
  iVar13 = cromu_sprintf();
  iVar14 = cromu_sprintf();
  iVar15 = cromu_sprintf();
  iVar16 = cromu_sprintf();
  iVar17 = cromu_sprintf();
  iVar17 = iVar1 + iVar2 + iVar3 + iVar4 + iVar5 + iVar6 + iVar7 + iVar8 + iVar9 + iVar10 + iVar11 +
           iVar12 + iVar13 + iVar14 + iVar15 + iVar16 + iVar17;
  iVar1 = cromu_sprintf(StatusBuf + iVar17,
                        "$-14s |  $8.2f   $8.2f   $8.2f   $8.2f   $8.2f     $8.2f          $8.2f     $8.2f\n"
                        ,&DAT_0001ef41,_DAT_0001efc0,_DAT_0001efc8,_DAT_0001efd0,_DAT_0001efd8,
                        _DAT_0001efe0,_DAT_0001ef6c,_DAT_0001ef7c,_DAT_0001ef84);
  iVar17 = iVar17 + iVar1;
  iVar1 = cromu_sprintf(StatusBuf + iVar17,
                        "$-14s |  $8.2f   $8.2f   $8.2f   $8.2f   $8.2f     $8.2f          $8.2f     $8.2f\n"
                        ,&DAT_0001f025,_DAT_0001f0a4,_DAT_0001f0ac,_DAT_0001f0b4,_DAT_0001f0bc,
                        _DAT_0001f0c4,_DAT_0001f050,_DAT_0001f060,_DAT_0001f068);
  cromu_sprintf(StatusBuf + iVar17 + iVar1,
                "$-14s |  $8.2f   $8.2f   $8.2f   $8.2f   $8.2f     $8.2f          $8.2f     $8.2f\n"
                ,&DAT_0001f109,_DAT_0001f188,_DAT_0001f190,_DAT_0001f198,_DAT_0001f1a0,_DAT_0001f1a8
                ,_DAT_0001f134,_DAT_0001f144,_DAT_0001f14c);
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  cromu_sprintf();
  return StatusBuf;
}

// Function: InitPlant
void InitPlant(void) {
  InitInfluent();
  InitHeadworks();
  InitHeadworksOutputValve();
  InitAsp();
  InitAspOutputValve();
  InitFilters();
  InitFilterOutputValve();
  InitDisinfection();
  InitDisinfectionOutputValve();
  InitEffluent();
  return;
}

