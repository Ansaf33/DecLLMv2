// Function: output_str
void output_str(int param_1,int param_2,undefined4 param_3) {
  float_to_str(*(double *)(output + (param_1 * 10 + param_2) * 8) - DAT_000166c0,param_3);
  return;
}

// Function: TGridAverage
longdouble
TGridAverage(int param_1,int param_2,uint param_3,uint param_4,int param_5,int param_6,int param_7) {
  uint local_24;
  uint local_20;
  double local_1c;
  double local_14;
  
  local_14 = 0.0;
  local_1c = 0.0;
  if (X < param_3 + param_5) {
    param_5 = X - param_3;
  }
  if (Y < param_4 + param_6) {
    param_6 = Y - param_4;
  }
  for (local_20 = param_3; local_20 < param_5 + param_3; local_20 = local_20 + 1) {
    for (local_24 = param_4; local_24 < param_6 + param_4; local_24 = local_24 + 1) {
      local_14 = local_14 + *(double *)((local_20 + X * local_24 + Y * X * param_7) * 8 + TGrid);
      local_1c = local_1c + 1.0;
    }
  }
  *(double *)(output + (param_1 * 10 + param_2) * 8) = local_14 / local_1c;
  return (longdouble)local_14 / (longdouble)local_1c;
}

// Function: GraphTemps
undefined4 GraphTemps(void) {
  int iVar1;
  uint uVar2;
  double dVar3;
  ulonglong local_5c;
  undefined4 local_44;
  undefined4 uStack_40;
  double local_34;
  uint local_28;
  uint local_24;
  uint local_20;
  uint local_1c;
  uint local_18;
  uint local_14;
  uint local_10;
  
  printf("\x1b[2J\x1b[H");
  local_5c = (ulonglong)X;
  local_5c._0_4_ = (uint)(longlong)ROUND((double)local_5c / DAT_000166c8 + 1.0);
  iVar1 = (uint)local_5c;
  local_5c = (ulonglong)Y;
  local_5c._0_4_ = (uint)(longlong)ROUND((double)local_5c / DAT_000166c8 + 1.0);
  for (local_18 = 0; local_18 < Z; local_18 = local_18 + 1) {
    uVar2 = local_18;
    printf("z: @d\n",local_18);
    local_14 = 0;
    for (local_28 = 0; dVar3 = (double)(ulonglong)uVar2, local_28 < Y;
        local_28 = local_28 + (uint)local_5c) {
      local_10 = 0;
      for (local_24 = 0; local_24 < X; local_24 = local_24 + iVar1) {
        uVar2 = (uint)local_5c;
        TGridAverage(local_10,local_14,local_24,local_28,iVar1,(uint)local_5c,local_18);
        local_10 = local_10 + 1;
      }
      local_14 = local_14 + 1;
    }
    for (local_1c = 0; local_1c < local_10; local_1c = local_1c + 1) {
      for (local_20 = 0; local_20 < local_14; local_20 = local_20 + 1) {
        dVar3 = *(double *)(output + (local_1c * 10 + local_20) * 8) - DAT_000166c0;
        local_34 = 0.0;
        if (0.0 <= dVar3) {
          if (0.0 < dVar3) {
            local_34 = DAT_000166d8;
          }
        }
        else {
          local_34 = DAT_000166d0;
        }
        local_44 = SUB84(dVar3,0);
        uStack_40 = (undefined4)((ulonglong)dVar3 >> 0x20);
        if (DAT_000166e0 < dVar3 + local_34) {
          if (DAT_000166e8 < dVar3 + local_34) {
            if (0.0 <= dVar3 + local_34) {
              if (DAT_000166c8 <= dVar3 + local_34) {
                if (_DAT_000166f0 <= dVar3 + local_34) {
                  printf(" @f ",local_44,uStack_40);
                }
                else {
                  printf("  @f ",local_44,uStack_40);
                }
              }
              else {
                printf("   @f ",local_44,uStack_40);
              }
            }
            else {
              printf("  @f ",local_44,uStack_40);
            }
          }
          else {
            printf(" @f ",local_44,uStack_40);
          }
        }
        else {
          printf("@f ",local_44,uStack_40);
        }
      }
      printf("\n",SUB84(dVar3,0));
    }
    printf("\n",SUB84(dVar3,0));
  }
  return 0;
}

