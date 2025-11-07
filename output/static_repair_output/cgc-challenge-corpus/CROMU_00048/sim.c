#include <stdbool.h> // For bool type
#include <stddef.h>  // For NULL
#include <stdint.h>  // For uint32_t, uint64_t

// Type definitions for clarity, mapping to standard C types
typedef unsigned int uint;
typedef unsigned char byte;
// Assuming 'undefined' is a generic byte, 'undefined4' is a 4-byte integer,
// and 'undefined8' is an 8-byte value (likely double given its usage with arithmetic)
// 'longdouble' maps to standard C 'long double'
typedef uint8_t undefined;
typedef uint32_t undefined4;
typedef double undefined8; // Based on usage in arithmetic operations

// Forward declarations for functions not provided in the snippet
long double GetValvePctFlow(char *param_1, unsigned char param_2);

// External global variables (declarations only, actual definitions would be elsewhere)
// Assuming DAT_XXXX addresses are base pointers to arrays or structures.
// Types are inferred from their usage in the provided code.

// Global arrays/buffers accessed with offsets
extern char DAT_0001ee4c[];
extern char DAT_0001e04c[];
extern char DAT_0001e7cc[];
extern char DAT_0001e14c[];

extern char *DAT_0001efe8[]; // Array of char pointers, 0xe4 is stride for entries
extern char DAT_0001efc0[];
extern char DAT_0001efc8[];
extern char DAT_0001efd0[];
extern char DAT_0001efd8[];
extern char DAT_0001efe0[];
extern char DAT_0001ef74[];
extern char DAT_0001ef6c[];
extern char DAT_0001ef7c[];
extern char DAT_0001ef84[];

extern char Filter[];      // Base for filter-related data structures
extern char Disinfection[];// Base for disinfection-related data structures

// Global variables (constants or state)
extern double DAT_00019468;
extern double DAT_00019470;
extern double DAT_00019478;
extern double DAT_00019480;
extern double DAT_00019488;
extern double DAT_00019490;
extern double DAT_00019498;
extern double DAT_000194a0;
extern double DAT_000194a8;
extern double DAT_000194b0;
extern double DAT_000194b8;
extern double DAT_000194c0;
extern double DAT_000194c8;
extern double DAT_000194d0;
extern double DAT_000194d8;
extern double DAT_000194e8;
extern double DAT_000194f8;
extern double DAT_00019508;
extern double DAT_00019518;
extern double DAT_00019520;
extern double DAT_00019530;
extern double DAT_00019528;
extern double DAT_00019510;
extern double DAT_00019500;
extern double DAT_000194f0;
extern double DAT_000194e0;
extern double DAT_00019540;
extern double DAT_00019538;
extern double DAT_00019550;
extern double DAT_00019548;
extern double DAT_00019558;
extern double DAT_00019560;
extern double DAT_00019568;

extern int NumAsp;
extern int NumFilters;
extern int NumDisinfection;

extern double DAT_0001e1c0;
extern double DAT_0001e1c8;
extern double DAT_0001e1d0;
extern double DAT_0001e1d8;
extern double DAT_0001e1e0;
extern double DAT_0001e1a0;
extern char *DAT_0001e1ac; // Pointer used as base address
extern unsigned char DAT_0001e196;

extern char *DAT_0001e2e8; // Pointer used as base address

extern double _DAT_0001e2c0;
extern double _DAT_0001e2c8;
extern double _DAT_0001e2d0;
extern double _DAT_0001e2d8;
extern double _DAT_0001e2e0;

// Global "valve" structures/buffers
extern char HeadworksOutputValve[];
extern char AspOutputValve[];
extern char FilterOutputValve[];
extern char DisinfectionOutputValve[];


// Function: FindValveOutput
uint FindValveOutput(char *param_1, int param_2) {
  unsigned char i; // Renamed local_5 to i

  if ((param_1 != NULL) && (*param_1 == 5) && (param_2 != 0)) {
    for (i = 0; i < 5; i++) {
      // Original: param_1 + ((char)i + 0x18) * 4 + 0xc
      // Simplified: param_1 + 108 + i * 4
      if (param_2 == *(int *)(param_1 + 108 + (unsigned int)i * 4)) {
        return i; // Implicit conversion from unsigned char to uint
      }
    }
  }
  return 0xFFFFFFFF;
}

// Function: SimulateValve
void SimulateValve(char *param_1) { // Changed param_1 to char*
  char *iVar1;
  unsigned char i; // Renamed local_5 to i

  // Initialize double values to 0.0
  *(double *)(param_1 + 0x80) = 0.0;
  *(double *)(param_1 + 0x88) = 0.0;
  *(double *)(param_1 + 0x90) = 0.0;
  *(double *)(param_1 + 0x98) = 0.0;
  *(double *)(param_1 + 0xa0) = 0.0;
  *(double *)(param_1 + 0x60) = 0.0;
  *(unsigned char *)(param_1 + 0x56) = 1;

  for (i = 0; i < 5; i++) {
    // Original: *(int *)(param_1 + 8 + (i + 0x28) * 4)
    // Simplified: *(char **)(param_1 + 168 + i * 4)
    // The value at this memory location is then used as a pointer, so it must be a char*
    iVar1 = *(char **)(param_1 + 168 + (unsigned int)i * 4);

    if (iVar1 != NULL) {
      *(double *)(param_1 + 0x80) += *(double *)(iVar1 + 0x80);
      *(double *)(param_1 + 0x88) += *(double *)(iVar1 + 0x88);
      *(double *)(param_1 + 0x90) += *(double *)(iVar1 + 0x90);
      *(double *)(param_1 + 0x98) += *(double *)(iVar1 + 0x98);
      *(double *)(param_1 + 0xa0) += *(double *)(iVar1 + 0xa0);
      *(double *)(param_1 + 0x60) += *(double *)(iVar1 + 0x60);
      if (*(char *)(iVar1 + 0x56) == 0) {
        *(unsigned char *)(param_1 + 0x56) = 0;
      }
    }
  }
}

// Function: CheckValveOutputs
uint32_t CheckValveOutputs(void) { // Changed undefined4 to uint32_t
  unsigned char i; // Renamed local_d to i
  double current_sum; // Renamed local_c to current_sum

  // Loop 1
  current_sum = 0.0;
  for (i = 0; i < 5; i++) {
    // Original: *(int *)(&DAT_0001ee4c + (i + 0x18) * 4)
    // Simplified: *(int *)(DAT_0001ee4c + (i + 24) * 4)
    if (*(int *)(DAT_0001ee4c + (unsigned int)(i + 24) * 4) != 0) {
      // Original: *(double *)(&DAT_0001ee4c + (i + 0x16) * 8)
      // Simplified: *(double *)(DAT_0001ee4c + (i + 22) * 8)
      current_sum += *(double *)(DAT_0001ee4c + (unsigned int)(i + 22) * 8);
    }
  }
  if (DAT_00019468 <= current_sum) {
    // Loop 2
    current_sum = 0.0;
    for (i = 0; i < 5; i++) {
      if (*(int *)(DAT_0001e04c + (unsigned int)(i + 24) * 4) != 0) {
        current_sum += *(double *)(DAT_0001e04c + (unsigned int)(i + 22) * 8);
      }
    }
    if (DAT_00019468 <= current_sum) {
      // Loop 3
      current_sum = 0.0;
      for (i = 0; i < 5; i++) {
        if (*(int *)(DAT_0001e7cc + (unsigned int)(i + 24) * 4) != 0) {
          current_sum += *(double *)(DAT_0001e7cc + (unsigned int)(i + 22) * 8);
        }
      }
      if (DAT_00019468 <= current_sum) {
        // Loop 4
        current_sum = 0.0;
        for (i = 0; i < 5; i++) {
          if (*(int *)(DAT_0001e14c + (unsigned int)(i + 24) * 4) != 0) {
            current_sum += *(double *)(DAT_0001e14c + (unsigned int)(i + 22) * 8);
          }
        }
        if (DAT_00019468 <= current_sum) {
          return 1;
        }
      }
    }
  }
  return 0; // Simplified return logic
}

// Function: SimulateAsp
void SimulateAsp(void) {
  char *pcVar2;
  unsigned char i = 0; // Renamed local_d to i
  long double pct_flow; // Intermediate variable for GetValvePctFlow result

  while ((i < NumAsp) &&
         ((pcVar2 = *(char **)(DAT_0001efe8 + (unsigned int)i * 0xe4)) != NULL) &&
         (*pcVar2 == 5)) {
    pct_flow = GetValvePctFlow(pcVar2, i); // Call once per iteration

    *(double *)(DAT_0001efc0 + (unsigned int)i * 0xe4) = (double)(pct_flow * *(double *)(pcVar2 + 0x80));
    *(double *)(DAT_0001efc8 + (unsigned int)i * 0xe4) = (double)(pct_flow * *(double *)(pcVar2 + 0x88));
    *(double *)(DAT_0001efd0 + (unsigned int)i * 0xe4) = (double)(pct_flow * *(double *)(pcVar2 + 0x90));
    *(double *)(DAT_0001efd8 + (unsigned int)i * 0xe4) = (double)(pct_flow * *(double *)(pcVar2 + 0x98));
    *(double *)(DAT_0001efe0 + (unsigned int)i * 0xe4) = (double)(pct_flow * *(double *)(pcVar2 + 0xa0));

    *(double *)(DAT_0001efd0 + (unsigned int)i * 0xe4) *= DAT_00019470;

    double dVar1 = DAT_00019478 *
                   *(double *)(DAT_0001ef74 + (unsigned int)i * 0xe4) *
                   (*(double *)(DAT_0001ef6c + (unsigned int)i * 0xe4) / DAT_00019468);
    double dVar3 = (*(double *)(DAT_0001efc8 + (unsigned int)i * 0xe4) +
                    *(double *)(DAT_0001efc8 + (unsigned int)i * 0xe4)) / DAT_00019480;

    if (dVar1 < dVar3) {
      *(double *)(DAT_0001efc8 + (unsigned int)i * 0xe4) = DAT_00019480 * ((dVar3 - dVar1) / DAT_00019488);
    } else {
      dVar1 -= dVar3;
      *(double *)(DAT_0001efc8 + (unsigned int)i * 0xe4) = 0.0;
      dVar3 = dVar1 / DAT_00019490;
      if (DAT_00019490 * (*(double *)(DAT_0001efd8 + (unsigned int)i * 0xe4) / DAT_00019480) <= dVar1) {
        *(double *)(DAT_0001efd8 + (unsigned int)i * 0xe4) = 0.0;
      } else {
        *(double *)(DAT_0001efd8 + (unsigned int)i * 0xe4) -= (DAT_00019480 * dVar1) / DAT_00019490;
      }
      dVar1 = DAT_00019498 * *(double *)(DAT_0001ef7c + (unsigned int)i * 0xe4);
      dVar3 = DAT_000194a0 * dVar3;
      if (dVar1 < dVar3) {
        *(double *)(DAT_0001efd8 + (unsigned int)i * 0xe4) = (dVar3 - dVar1) + *(double *)(DAT_0001efd8 + (unsigned int)i * 0xe4);
      } else {
        *(double *)(DAT_0001efc8 + (unsigned int)i * 0xe4) = DAT_00019480 * (dVar1 - dVar3) + *(double *)(DAT_0001efc8 + (unsigned int)i * 0xe4);
      }
      dVar1 = DAT_000194a8 * *(double *)(DAT_0001ef84 + (unsigned int)i * 0xe4);
      dVar3 = DAT_000194b0 * (*(double *)(DAT_0001efe0 + (unsigned int)i * 0xe4) / DAT_00019480);
      if (dVar1 < dVar3) {
        *(double *)(DAT_0001efe0 + (unsigned int)i * 0xe4) = DAT_00019480 * ((dVar3 - dVar1) / DAT_000194b0);
      } else {
        *(double *)(DAT_0001efe0 + (unsigned int)i * 0xe4) = 0.0;
        *(double *)(DAT_0001efc8 + (unsigned int)i * 0xe4) = DAT_00019480 * (dVar1 - dVar3) + *(double *)(DAT_0001efc8 + (unsigned int)i * 0xe4);
      }
    }
    i++;
  }
}

// Function: SimulateFilter
void SimulateFilter(void) {
  char *pcVar2;
  unsigned char i = 0; // Renamed local_d to i
  long double pct_flow; // Intermediate variable for GetValvePctFlow result

  while ((i < NumFilters) &&
         ((pcVar2 = *(char **)(Filter + (unsigned int)i * 0xe4 + 0xa8)) != NULL) &&
         (*pcVar2 == 5)) {
    pct_flow = GetValvePctFlow(pcVar2, i); // Call once per iteration

    *(double *)(Filter + (unsigned int)i * 0xe4 + 0x80) = (double)(pct_flow * *(double *)(pcVar2 + 0x80));
    *(double *)(Filter + (unsigned int)i * 0xe4 + 0x88) = (double)(pct_flow * *(double *)(pcVar2 + 0x88));
    *(double *)(Filter + (unsigned int)i * 0xe4 + 0x90) = (double)(pct_flow * *(double *)(pcVar2 + 0x90));
    *(double *)(Filter + (unsigned int)i * 0xe4 + 0x98) = (double)(pct_flow * *(double *)(pcVar2 + 0x98));
    *(double *)(Filter + (unsigned int)i * 0xe4 + 0xa0) = (double)(pct_flow * *(double *)(pcVar2 + 0xa0));

    double dVar1 = ((DAT_000194b8 * *(double *)(Filter + (unsigned int)i * 0xe4 + 0x80)) / DAT_000194c0) /
                   (DAT_000194c8 * (double)(unsigned char)Filter[(unsigned int)i * 0xe4 + 0x54]);

    if (dVar1 <= DAT_000194d0) {
      if (dVar1 < DAT_000194d8) {
        if (dVar1 < DAT_000194e8) {
          if (dVar1 < DAT_000194f8) {
            if (dVar1 < DAT_00019508) {
              if (dVar1 < DAT_00019518) {
                if (dVar1 < DAT_00019520) {
                  if (dVar1 < 1.0) {
                    *(double *)(Filter + (unsigned int)i * 0xe4 + 0x90) = 0.0;
                  } else {
                    *(double *)(Filter + (unsigned int)i * 0xe4 + 0x90) *= DAT_00019530;
                  }
                } else {
                  *(double *)(Filter + (unsigned int)i * 0xe4 + 0x90) *= DAT_00019528;
                }
              } else {
                *(double *)(Filter + (unsigned int)i * 0xe4 + 0x90) *= DAT_00019470;
              }
            } else {
              *(double *)(Filter + (unsigned int)i * 0xe4 + 0x90) *= DAT_00019510;
            }
          } else {
            *(double *)(Filter + (unsigned int)i * 0xe4 + 0x90) *= DAT_00019500;
          }
        } else {
          *(double *)(Filter + (unsigned int)i * 0xe4 + 0x90) *= DAT_000194f0;
        }
      } else {
        *(double *)(Filter + (unsigned int)i * 0xe4 + 0x90) *= DAT_000194e0;
      }
    }
    i++;
  }
}

// Function: SimulateDisinfection
void SimulateDisinfection(void) {
  char *pcVar2;
  unsigned char i = 0; // Renamed local_d to i
  long double pct_flow; // Intermediate variable for GetValvePctFlow result

  while ((i < NumDisinfection) &&
         ((pcVar2 = *(char **)(Disinfection + (unsigned int)i * 0xe4 + 0xa8)) != NULL) &&
         (*pcVar2 == 5)) {
    pct_flow = GetValvePctFlow(pcVar2, i); // Call once per iteration

    *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x80) = (double)(pct_flow * *(double *)(pcVar2 + 0x80));
    *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x88) = (double)(pct_flow * *(double *)(pcVar2 + 0x88));
    *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x90) = (double)(pct_flow * *(double *)(pcVar2 + 0x90));
    *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x98) = (double)(pct_flow * *(double *)(pcVar2 + 0x98));
    *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0xa0) = (double)(pct_flow * *(double *)(pcVar2 + 0xa0));

    if (DAT_00019540 <=
        DAT_00019538 /
        ((DAT_000194b8 * *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x80)) / DAT_000194c0)) {
      Disinfection[(unsigned int)i * 0xe4 + 0x56] = 1;
    } else {
      Disinfection[(unsigned int)i * 0xe4 + 0x56] = 0;
    }

    double local_1c = DAT_00019480 *
                      DAT_00019550 *
                      DAT_00019548 *
                      ((DAT_00019518 * *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x58)) /
                      DAT_00019468);

    if (0.0 < *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x98)) {
      if (DAT_00019558 * *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x98) < local_1c) {
        local_1c -= DAT_00019558 * *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x98);
        *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x98) = 0.0;
      } else {
        *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x98) -= local_1c / DAT_00019558;
        local_1c = 0.0;
      }
    }
    local_1c /= (DAT_00019560 * *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x80));
    if (local_1c - DAT_000194e8 < 0.0) {
      *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x60) = 0.0;
    } else {
      *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x60) = local_1c - DAT_000194e8;
    }
    if ((*(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x60) < 1.0) ||
       (DAT_000194f8 < *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x60))) {
      Disinfection[(unsigned int)i * 0xe4 + 0x56] = 0;
    }
    *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x60) =
         DAT_00019560 *
         *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x80) *
         *(double *)(Disinfection + (unsigned int)i * 0xe4 + 0x60);
    i++;
  }
}

// Function: SimulateEffluent
void SimulateEffluent(void) {
  char *iVar2 = DAT_0001e1ac; // Changed iVar2 to char* and assigned directly
  long double pct_flow; // Intermediate variable for GetValvePctFlow result

  if (iVar2 != NULL) {
    pct_flow = GetValvePctFlow(&DisinfectionOutputValve[0], 0); // Call once

    *(double *)(iVar2 + 0x80) = (double)(pct_flow * DAT_0001e1c0);
    *(double *)(iVar2 + 0x88) = (double)(pct_flow * DAT_0001e1c8);
    *(double *)(iVar2 + 0x90) = (double)(pct_flow * DAT_0001e1d0);
    *(double *)(iVar2 + 0x98) = (double)(pct_flow * DAT_0001e1d8);
    *(double *)(iVar2 + 0xa0) = (double)(pct_flow * DAT_0001e1e0);
    *(double *)(iVar2 + 0x60) = (double)(pct_flow * DAT_0001e1a0);
    *(unsigned char *)(iVar2 + 0x56) = DAT_0001e196;
  }
}

// Function: SimulateHeadworks
void SimulateHeadworks(void) {
  if (DAT_0001e2e8 != NULL) {
    _DAT_0001e2c0 = *(double *)(DAT_0001e2e8 + 0x80);
    _DAT_0001e2c8 = *(double *)(DAT_0001e2e8 + 0x88);
    _DAT_0001e2d0 = DAT_00019568 * *(double *)(DAT_0001e2e8 + 0x90);
    _DAT_0001e2d8 = *(double *)(DAT_0001e2e8 + 0x98);
    _DAT_0001e2e0 = *(double *)(DAT_0001e2e8 + 0xa0);
  }
}

// Function: RunSimStep
bool RunSimStep(void) {
  uint32_t check_result = CheckValveOutputs(); // Renamed cVar1

  if (check_result != 0) {
    SimulateHeadworks();
    SimulateValve(&HeadworksOutputValve[0]); // Pass address of first element
    SimulateAsp();
    SimulateValve(&AspOutputValve[0]); // Pass address of first element
    SimulateFilter();
    SimulateValve(&FilterOutputValve[0]); // Pass address of first element
    SimulateDisinfection();
    SimulateValve(&DisinfectionOutputValve[0]); // Pass address of first element
    SimulateEffluent();
  }
  return check_result != 0;
}