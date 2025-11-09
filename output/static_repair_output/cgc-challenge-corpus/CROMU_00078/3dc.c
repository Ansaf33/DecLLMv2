#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>   // For cos, sin, round, fmax, fmin
#include <limits.h> // For SHRT_MAX, SHRT_MIN

// --- Custom types from decompiler output ---

// PixelData structure (10 bytes)
// XYZ coordinates (3 int16_t) + RGBA (4 uint8_t)
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} PixelData;

// Function pointer type for RunTask
typedef void (*TaskFunction)(uint32_t, int);

// --- External function declarations (placeholders) ---
// These functions are not provided in the snippet, but are used.
// Their signatures are inferred from context.
extern void seed_prng(uint32_t seed);
extern uint16_t prng(void); // prng returns undefined2, so uint16_t
extern void receive_bytes(void *buffer, size_t size);

// --- Global variables (placeholders) ---
// Inferred from decompiler DAT_ addresses.
// Assuming they are static constants or global variables.
static uint32_t _DAT_4347c000 = 0x12345678; // Example PRNG seed
static double DAT_00016070 = 100.0;        // Example scale factor base (e.g., for percentage)
static double _DAT_00016080 = M_PI;        // Example PI constant for degree_to_radian
static double DAT_00016078 = 180.0;        // Example 180.0 constant for degree_to_radian

// --- Helper functions (re-defined based on usage and inferred intent) ---

// Redefined based on how they are called (taking two doubles)
// and their original return type usage.
int multiply(double val1, double val2) {
  return (int)round(val1 * val2);
}

short divide(double val1, double val2) {
  return (short)round(val1 / val2);
}

// degree_to_radian should return a double representing radians
double degree_to_radian(int degrees) {
  return (double)degrees * divide(_DAT_00016080, DAT_00016078);
}

// Function: Push
void Push(void **param_1, void *param_2) {
  if ((param_2 != NULL) && (param_1 != NULL)) {
    void *dest = malloc(sizeof(PixelData)); // Allocate for PixelData struct
    if (dest == NULL) {
        return; // Handle malloc failure
    }
    memcpy(dest, param_2, sizeof(PixelData)); // Copy 10 bytes (PixelData size)
    for (unsigned int i = 0; i < 0x19a; ++i) {
      if (param_1[i] == NULL) { // Check for a NULL pointer slot
        param_1[i] = dest;
        return;
      }
    }
    // If no space found, free the allocated memory to avoid a leak
    free(dest);
  }
}

// Function: RunTask
void RunTask(uint32_t *param_1, TaskFunction param_2, int param_3) {
  for (int i = 0; i < 0x199; ++i) {
    param_2(param_1[i], param_3);
  }
}

// Function: ReadFile
void ReadFile(void **param_1) {
  seed_prng(_DAT_4347c000);
  for (int i = 0x1000; i > 0; i -= 10) {
    PixelData *pixel_data = (PixelData *)malloc(sizeof(PixelData));
    if (pixel_data == NULL) {
        // Handle malloc failure, e.g., break the loop
        break;
    }
    pixel_data->x = prng();
    pixel_data->y = prng();
    pixel_data->z = prng();
    pixel_data->r = (uint8_t)prng();
    pixel_data->g = (uint8_t)prng();
    pixel_data->b = (uint8_t)prng();
    pixel_data->a = (uint8_t)prng();
    Push(param_1, pixel_data);
    free(pixel_data); // Free the temporary buffer, Push copies its content.
  }
}

// Function: NewFile
void NewFile(void **param_1, const uint8_t *param_2) {
  printf("Please submit your new file data (%d bytes):\n", 0xffa); // Corrected format specifier
  receive_bytes((void *)param_2, 0xffa);
  for (unsigned int i = 0; i < 0xffa / sizeof(PixelData); ++i) {
    PixelData *dest_pixel = (PixelData *)param_1[i];
    if (dest_pixel == NULL) {
        // Handle null destination, e.g., allocate or skip
        continue;
    }
    // Copy the entire PixelData structure if source is contiguous
    memcpy(dest_pixel, param_2 + i * sizeof(PixelData), sizeof(PixelData));
  }
  printf("New file loaded\n");
}

// Function: ShowPixel
void ShowPixel(const PixelData *param_1) {
  printf("XYZ:  (%d, %d, %d)\n", (int)param_1->x, (int)param_1->y, (int)param_1->z); // Corrected format specifiers
  printf("RGBA: (#%02x%02x%02x%02x)\n", (unsigned int)param_1->r, // Corrected format specifiers
         (unsigned int)param_1->g, (unsigned int)param_1->b,
         (unsigned int)param_1->a);
  printf("\n");
}

// Function: CheckFile
void CheckFile(void **param_1, int param_2) {
  if (param_1 != NULL) {
    for (int i = 0; i < param_2; ++i) {
      const PixelData *pixel = (const PixelData *)param_1[i];
      if (pixel == NULL) {
          printf("Pixel %d is NULL\n", i);
          continue;
      }
      printf("XYZ:  (%d, %d, %d)\n", (int)pixel->x, (int)pixel->y, (int)pixel->z); // Corrected format specifiers
      printf("RGBA: (#%02x%02x%02x%02x)\n", (unsigned int)pixel->r, // Corrected format specifiers
             (unsigned int)pixel->g, (unsigned int)pixel->b,
             (unsigned int)pixel->a);
      printf("\n");
    }
  }
}

// Function: RotateX
void RotateX(PixelData *param_1, int param_2) {
  double angle_rad = degree_to_radian(param_2);
  double cos_angle = cos(angle_rad);
  double sin_angle = sin(angle_rad);

  double old_y = (double)param_1->y;
  double old_z = (double)param_1->z;

  param_1->y = (int16_t)round(old_y * cos_angle - old_z * sin_angle);
  param_1->z = (int16_t)round(old_y * sin_angle + old_z * cos_angle);
}

// Function: RotateY
void RotateY(PixelData *param_1, int param_2) {
  double angle_rad = degree_to_radian(param_2);
  double cos_angle = cos(angle_rad);
  double sin_angle = sin(angle_rad);

  double old_x = (double)param_1->x;
  double old_z = (double)param_1->z;

  param_1->x = (int16_t)round(old_x * cos_angle + old_z * sin_angle);
  param_1->z = (int16_t)round(old_z * cos_angle - old_x * sin_angle);
}

// Function: RotateZ
void RotateZ(PixelData *param_1, int param_2) {
  double angle_rad = degree_to_radian(param_2);
  double cos_angle = cos(angle_rad);
  double sin_angle = sin(angle_rad);

  double old_x = (double)param_1->x;
  double old_y = (double)param_1->y;

  param_1->x = (int16_t)round(old_x * cos_angle - old_y * sin_angle);
  param_1->y = (int16_t)round(old_x * sin_angle + old_y * cos_angle);
}

// Function: SkewX
void SkewX(PixelData *param_1, short param_2) {
  param_1->x += param_2;
}

// Function: SkewY
void SkewY(PixelData *param_1, short param_2) {
  param_1->y += param_2;
}

// Function: SkewZ
void SkewZ(PixelData *param_1, short param_2) {
  param_1->z += param_2;
}

// Function: Scale
void Scale(PixelData *param_1, short param_2) {
  if ((0 < param_2) && (param_2 < 0xc9)) { // 0xc9 is 201
    double scale_factor = (double)param_2 / DAT_00016070;

    int new_x = multiply((double)param_1->x, scale_factor);
    int new_y = multiply((double)param_1->y, scale_factor);
    int new_z = multiply((double)param_1->z, scale_factor);

    // Clamping to short range
    param_1->x = (int16_t)fmax(SHRT_MIN, fmin(SHRT_MAX, new_x));
    param_1->y = (int16_t)fmax(SHRT_MIN, fmin(SHRT_MAX, new_y));
    param_1->z = (int16_t)fmax(SHRT_MIN, fmin(SHRT_MAX, new_z));
  }
}

// Function: Brightness
void Brightness(PixelData *param_1, short param_2) {
  if ((-0x100 < param_2) && (param_2 < 0x100)) { // -256 < param_2 < 256
    int r_val = (unsigned char)param_1->r + param_2;
    int g_val = (unsigned char)param_1->g + param_2;
    int b_val = (unsigned char)param_1->b + param_2;

    // Clamping to 0-255 range
    param_1->r = (uint8_t)fmax(0, fmin(255, r_val));
    param_1->g = (uint8_t)fmax(0, fmin(255, g_val));
    param_1->b = (uint8_t)fmax(0, fmin(255, b_val));
  }
}

// Function: Opacity
void Opacity(PixelData *param_1, uint8_t param_2) {
  param_1->a = param_2;
}