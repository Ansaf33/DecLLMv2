#include <stdlib.h>  // For rand()
#include <math.h>    // For log(), sqrt(), round()
#include <stdbool.h> // For bool type

// Global static variables as inferred from DAT_000...
// These variables are assumed to be defined elsewhere and initialized.
// Declared as 'static' to give them internal linkage, mimicking typical compiler output.

// Coefficients and constants
static long double DAT_000164c8;
static long double DAT_000164c0;
static double DAT_000164d0;
static double DAT_000164d8;
static double DAT_000164e0;
static double DAT_000164e8;
static double DAT_000164f0;
static double DAT_000164f8;
static double DAT_00016500;
static double DAT_00016508;
static double DAT_00016510;
static double DAT_00016518;
static double DAT_00016520;
static double DAT_00016528;
static double DAT_00016530;
static double DAT_00016538;
static double DAT_00016540;
static double DAT_00016548;
static double DAT_00016550; // Corrected from _DAT_00016550

// State variables (filter taps, accumulators, etc.)
static double DAT_00019028;
static double DAT_00019030;
static double DAT_00019038;
static double DAT_00019040;
static double DAT_00019048;
static double DAT_00019050;

static double DAT_00019068;
static double DAT_00019070;
static double DAT_00019078;
static double DAT_00019080;
static double DAT_00019088;
static double DAT_00019090;

static double DAT_000190a8;
static double DAT_000190b0;
static double DAT_000190b8;
static double DAT_000190c0;
static double DAT_000190c8;
static double DAT_000190d0;

static double DAT_000190e8;
static double DAT_000190f0;
static double DAT_000190f8;
static double DAT_00019100;
static double DAT_00019108;
static double DAT_00019110;

static double DAT_00019128;
static double DAT_00019130;
static double DAT_00019138;

static double DAT_00019148;
static double DAT_00019150;
static double DAT_00019158;

static double DAT_00019168;
static double DAT_00019170;
static double DAT_00019178;

static double DAT_00019188;
static double DAT_00019190;
static double DAT_00019198;


// Function: get_rand_uniform
long double get_rand_uniform(void) {
  // Reduced intermediate variables by directly using rand() in the expression.
  return DAT_000164c8 * (DAT_000164c0 * (long double)(rand() >> 4) + (long double)(rand() >> 5));
}

// Function: add_awgn_to_sample
char add_awgn_to_sample(char param_1) {
  long double u1, u2;
  double v1, v2, s; // Renamed for clarity, reduced from dVar1, dVar7
  double noise_magnitude; // Renamed from dVar5
  double std_dev_factor = DAT_000164d8; // Renamed from dVar7 (which was a re-use of Box-Muller 's' variable)

  do {
    u1 = get_rand_uniform();
    u2 = get_rand_uniform();
    v1 = (2.0 * u1) - 1.0;
    v2 = (2.0 * u2) - 1.0;
    s = v1 * v1 + v2 * v2;
  } while (1.0 <= s || s == 0.0); // Added s == 0.0 check to prevent log(0) and division by zero

  noise_magnitude = sqrt((DAT_000164d0 * log(s)) / s);

  // The original code had a redundant calculation (dVar6 and an unassigned sqrt). Removed.

  char noisy_sample_component = (char)(short)round(std_dev_factor * v1 * noise_magnitude);
  short temp_sum = (short)param_1 + noisy_sample_component;

  // Clamping logic for char (-127 to 127)
  if (temp_sum > 127) {
    return 127;
  } else if (temp_sum < -127) {
    return -127;
  } else {
    return (char)temp_sum;
  }
}

// Function: receive_sample
bool receive_sample(char param_1) {
  // These dVar variables store the state before the shift operations.
  // They are necessary to correctly calculate the new state,
  // thus not considered "intermediate" in a reducible sense for this pattern.
  double dVar1 = DAT_00019028;
  double dVar2 = DAT_00019038;
  double dVar3 = DAT_00019048;
  double dVar4 = DAT_00019068;
  double dVar5 = DAT_00019070;
  double dVar6 = DAT_00019078;
  double dVar7 = DAT_00019080;
  double dVar8 = DAT_00019088;
  double dVar9 = DAT_000190a8;
  double dVar10 = DAT_000190b8;
  double dVar11 = DAT_000190c8;
  double dVar12 = DAT_000190e8;
  double dVar13 = DAT_000190f0;
  double dVar14 = DAT_000190f8;
  double dVar15 = DAT_00019100;
  double dVar16 = DAT_00019108;
  double dVar17 = DAT_00019128;
  double dVar18 = DAT_00019130;
  double dVar19 = DAT_00019138;
  double dVar20 = DAT_00019148;
  double dVar21 = DAT_00019150;
  double dVar22 = DAT_00019168;
  double dVar23 = DAT_00019170;
  double dVar24 = DAT_00019178;
  double dVar25 = DAT_00019188;
  double dVar26 = DAT_00019190;
  
  char cVar27 = add_awgn_to_sample(param_1); // param_1 is already char, no need for (int) cast

  // Shift and update operations
  // Removed redundant xv... and yv... assignments as their values were already copied to dVar variables.
  DAT_00019028 = DAT_00019030;
  DAT_00019030 = DAT_00019038;
  DAT_00019038 = DAT_00019040;
  DAT_00019040 = DAT_00019048;
  DAT_00019048 = DAT_00019050;
  DAT_00019050 = (double)(short)cVar27 / DAT_000164e0;

  DAT_00019068 = DAT_00019070;
  DAT_00019070 = DAT_00019078;
  DAT_00019078 = DAT_00019080;
  DAT_00019080 = DAT_00019088;
  DAT_00019088 = DAT_00019090;
  // Update DAT_00019090 using the 'old' values stored in dVar variables
  DAT_00019090 = DAT_00016518 * DAT_00019090 +
                 DAT_00016510 * dVar8 +
                 DAT_00016508 * dVar7 +
                 DAT_00016500 * dVar6 +
                 DAT_000164f8 * dVar5 +
                 DAT_000164f0 * dVar4 +
                 DAT_000164e8 * (dVar2 - dVar3) + (DAT_00019050 - dVar1);

  DAT_000190a8 = DAT_000190b0;
  DAT_000190b0 = DAT_000190b8;
  DAT_000190b8 = DAT_000190c0;
  DAT_000190c0 = DAT_000190c8;
  DAT_000190c8 = DAT_000190d0;
  DAT_000190d0 = (double)(short)cVar27 / DAT_00016520;

  DAT_000190e8 = DAT_000190f0;
  DAT_000190f0 = DAT_000190f8;
  DAT_000190f8 = DAT_00019100;
  DAT_00019100 = DAT_00019108;
  DAT_00019108 = DAT_00019110;
  // Update DAT_00019110 using the 'old' values stored in dVar variables
  // Removed terms multiplied by 0.0 for reduction and clarity.
  DAT_00019110 = DAT_00016530 * dVar16 +
                 DAT_00016528 * dVar14 +
                 DAT_000164f0 * dVar12 +
                 DAT_000164e8 * (dVar10 - dVar11) + (DAT_000190d0 - dVar9);

  DAT_00019128 = DAT_00019130;
  DAT_00019130 = DAT_00019138;
  DAT_00019138 = (DAT_00019090 * DAT_00019090) / DAT_00016538;

  DAT_00019148 = DAT_00019150;
  DAT_00019150 = DAT_00019158;
  // Update DAT_00019158 using the 'old' values stored in dVar variables
  DAT_00019158 = DAT_00016550 * DAT_00019158 +
                 DAT_00016548 * dVar21 +
                 DAT_00016540 * dVar20 + DAT_000164e8 * (dVar19 + dVar18) + DAT_00019138 + dVar17;

  DAT_00019168 = DAT_00019170;
  DAT_00019170 = DAT_00019178;
  DAT_00019178 = (DAT_00019110 * DAT_00019110) / DAT_00016538;

  DAT_00019188 = DAT_00019190;
  DAT_00019190 = DAT_00019198;
  // Update DAT_00019198 using the 'old' values stored in dVar variables
  DAT_00019198 = DAT_00016550 * DAT_00019198 +
                 DAT_00016548 * dVar26 +
                 DAT_00016540 * dVar25 + DAT_000164e8 * (dVar24 + dVar23) + DAT_00019178 + dVar22;
  
  return DAT_00019158 < DAT_00019198;
}