#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // For malloc, free, abs

// Type definitions to match common decompiled patterns
typedef uint8_t byte;
typedef uint8_t undefined;
typedef uint16_t ushort;
typedef uint32_t undefined4; // Used for Context* in ConnectPoints

// Context structure for Get/SetColor and ConnectPoints
// `param_1` in Get/SetColor (int) and ConnectPoints (undefined4) is a pointer to this.
typedef struct {
    uint8_t  _pad0[2];        // Placeholder, offset 0-1
    ushort   pitch;           // Offset 2: `*(ushort *)(ctx + 2)` - Bytes per scanline
    uint8_t  _pad1[4];        // Placeholder, offset 4-7
    uint8_t* buffers[256];    // Offset 8: `*(int *)(ctx + 8 + (uint)buffer_idx * 4)`
                               // Array of pointers to pixel data for different "layers" or "color channels".
                               // Assuming `param_4` (byte) is an index into this array.
} Context;

// Canvas structure for Paint functions
// `param_1` in Paint functions (ushort*) is a pointer to this.
// Based on usage and typical graphics APIs, (width, height) is standard.
// The original code's bounds checks seemed to swap width/height, but they've been corrected to
// standard (x < width, y < height) in the function implementations.
typedef struct {
    ushort width;
    ushort height;
} Canvas;

// Function prototypes
// Note: ConnectPoints' 8th parameter was unused in the original snippet and has been removed from its signature and calls.
int GetColor(Context* ctx, ushort y, ushort x, byte buffer_idx);
undefined GetColorIndex(Context* ctx, ushort y, ushort x, byte buffer_idx);
void SetColor(Context* ctx, ushort y, ushort x, byte buffer_idx, undefined value);

void ConnectPoints(Context* ctx, byte buffer_idx, undefined value, ushort x1, ushort y1, ushort x2, ushort y2);

void PaintTriangle(Canvas* canvas, byte buffer_idx, undefined value, char fill_flag, ushort* points);
void PaintRectangle(Canvas* canvas, byte buffer_idx, undefined value, char fill_flag, ushort* rect_params);
void PaintSquare(Canvas* canvas, byte buffer_idx, undefined value, char fill_flag, ushort* square_params);
void PaintLine(Canvas* canvas, byte buffer_idx, undefined value, byte unused_param, ushort* line_params);
void PaintCircle(Canvas* canvas, byte buffer_idx, undefined value, char fill_flag, ushort* circle_params);
void PaintSpray(Canvas* canvas, byte buffer_idx, undefined value, byte unused_param, ushort* spray_params);


// Function: GetColor
int GetColor(Context* ctx, ushort y, ushort x, byte buffer_idx) {
  // The original return statement `param_1 + (uint)*(byte *)(...) * 3 + 0x20` is ambiguous.
  // Given `GetColorIndex` returns the raw byte, `GetColor` likely performs a palette lookup
  // or similar transformation. Without a palette definition, the `* 3 + 0x20` part
  // cannot be meaningfully replicated. For now, it returns the raw pixel value as an int.
  return (int)*(ctx->buffers[buffer_idx] + x + (uint32_t)y * ctx->pitch);
}

// Function: GetColorIndex
undefined GetColorIndex(Context* ctx, ushort y, ushort x, byte buffer_idx) {
  return *(ctx->buffers[buffer_idx] + x + (uint32_t)y * ctx->pitch);
}

// Function: SetColor
void SetColor(Context* ctx, ushort y, ushort x, byte buffer_idx, undefined value) {
  uint8_t* buffer = ctx->buffers[buffer_idx];
  if (buffer != NULL) {
    *(buffer + x + (uint32_t)y * ctx->pitch) = value;
  }
}

// Function: ConnectPoints (Bresenham-like line drawing)
void ConnectPoints(Context* ctx, byte buffer_idx, undefined value, ushort x1, ushort y1, ushort x2, ushort y2) {
  short sx, sy;
  int dx_abs, dy_abs;
  int longest_axis_len;

  if (x1 < x2) {
    dx_abs = (uint)x2 - (uint)x1;
    sx = 1;
  } else {
    dx_abs = (uint)x1 - (uint)x2;
    sx = -1;
  }
  if (y1 < y2) {
    dy_abs = (uint)y2 - (uint)y1;
    sy = 1;
  } else {
    dy_abs = (uint)y1 - (uint)y2;
    sy = -1;
  }

  longest_axis_len = (dx_abs < dy_abs) ? dy_abs : dx_abs;

  int error_x = 0;
  int error_y = 0;

  for (int i = 0; i <= longest_axis_len + 1; ++i) {
    SetColor(ctx, y1, x1, buffer_idx, value);
    error_x += dx_abs;
    error_y += dy_abs;

    if (longest_axis_len < error_x) {
      x1 += sx;
      error_x -= longest_axis_len;
    }
    if (longest_axis_len < error_y) {
      y1 += sy;
      error_y -= longest_axis_len;
    }
  }
}

// Function: PaintTriangle
void PaintTriangle(Canvas* canvas, byte buffer_idx, undefined value, char fill_flag, ushort* points) {
  // points: {x1, y1, x2, y2, x3, y3} => points[0]-points[5]
  ushort p1_x = points[0], p1_y = points[1];
  ushort p2_x = points[2], p2_y = points[3];
  ushort p3_x = points[4], p3_y = points[5];

  // Bounds check: all points must be within canvas dimensions
  if ((p1_x < canvas->width && p1_y < canvas->height &&
       p2_x < canvas->width && p2_y < canvas->height &&
       p3_x < canvas->width && p3_y < canvas->height)) {

    if (fill_flag == 0) { // Outline
      ConnectPoints((Context*)canvas, buffer_idx, value, p1_x, p1_y, p2_x, p2_y);
      ConnectPoints((Context*)canvas, buffer_idx, value, p3_x, p3_y, p1_x, p1_y);
      ConnectPoints((Context*)canvas, buffer_idx, value, p2_x, p2_y, p3_x, p3_y);
    } else { // Fill
      // Sort points by Y-coordinate, then by X-coordinate for ties
      ushort x[3] = {p1_x, p2_x, p3_x};
      ushort y[3] = {p1_y, p2_y, p3_y};

      for (int i = 0; i < 3; ++i) {
          for (int j = i + 1; j < 3; ++j) {
              if (y[i] > y[j] || (y[i] == y[j] && x[i] > x[j])) {
                  ushort temp_y = y[i]; y[i] = y[j]; y[j] = temp_y;
                  ushort temp_x = x[i]; x[i] = x[j]; x[j] = temp_x;
              }
          }
      }

      ushort p_top_x = x[0], p_top_y = y[0];
      ushort p_mid_x = x[1], p_mid_y = y[1];
      ushort p_bot_x = x[2], p_bot_y = y[2];

      // Handle horizontal top edge explicitly if it exists
      if (p_top_y == p_mid_y) {
          ushort min_x = (p_top_x < p_mid_x) ? p_top_x : p_mid_x;
          ushort max_x = (p_top_x > p_mid_x) ? p_top_x : p_mid_x;
          if (min_x < max_x) {
              ConnectPoints((Context*)canvas, buffer_idx, value, min_x, p_top_y, max_x - 1, p_top_y);
          }
      }

      // First segment: from p_top_y to p_mid_y (exclusive of p_mid_y if not horizontal)
      if (p_top_y < p_mid_y) {
          int dx_top_bot = (int)p_bot_x - (int)p_top_x;
          int dy_top_bot = (int)p_bot_y - (int)p_top_y;
          int dx_top_mid = (int)p_mid_x - (int)p_top_x;
          int dy_top_mid = (int)p_mid_y - (int)p_top_y;

          int current_x_tb_accumulator = 0;
          int current_x_tm_accumulator = 0;

          ushort loop_end_y = (p_mid_y == p_bot_y) ? p_mid_y : p_mid_y -1;
          if (p_top_y == p_mid_y) loop_end_y = p_top_y;

          for (ushort current_y = p_top_y; current_y <= loop_end_y; ++current_y) {
              if (dy_top_bot == 0 || dy_top_mid == 0) continue;

              ushort x_tb = p_top_x + current_x_tb_accumulator / dy_top_bot;
              ushort x_tm = p_top_x + current_x_tm_accumulator / dy_top_mid;

              current_x_tb_accumulator += dx_top_bot;
              current_x_tm_accumulator += dx_top_mid;

              ushort scan_x1 = x_tm;
              ushort scan_x2 = x_tb;
              if (x_tb < x_tm) {
                  scan_x1 = x_tb;
                  scan_x2 = x_tm;
              }

              if (scan_x1 < scan_x2) {
                  ConnectPoints((Context*)canvas, buffer_idx, value, scan_x1, current_y, scan_x2 - 1, current_y);
              }
          }
      }

      // Handle horizontal mid/bot edge explicitly if it exists
      if (p_mid_y == p_bot_y) {
          ushort min_x = (p_mid_x < p_bot_x) ? p_mid_x : p_bot_x;
          ushort max_x = (p_mid_x > p_bot_x) ? p_mid_x : p_bot_x;
          if (min_x < max_x) {
              ConnectPoints((Context*)canvas, buffer_idx, value, min_x, p_mid_y, max_x - 1, p_mid_y);
          }
      }

      // Second segment: from p_mid_y to p_bot_y (inclusive)
      if (p_mid_y < p_bot_y) {
          int dx_top_bot = (int)p_bot_x - (int)p_top_x;
          int dy_top_bot = (int)p_bot_y - (int)p_top_y;
          int dx_mid_bot = (int)p_bot_x - (int)p_mid_x;
          int dy_mid_bot = (int)p_bot_y - (int)p_mid_y;

          int current_x_tb_accumulator = (p_mid_y - p_top_y) * dx_top_bot;
          int current_x_mb_accumulator = (p_mid_y - p_mid_y) * dx_mid_bot;

          for (ushort current_y = p_mid_y; current_y <= p_bot_y; ++current_y) {
              if (dy_top_bot == 0 || dy_mid_bot == 0) continue;

              ushort x_tb = p_top_x + current_x_tb_accumulator / dy_top_bot;
              ushort x_mb = p_mid_x + current_x_mb_accumulator / dy_mid_bot;

              current_x_tb_accumulator += dx_top_bot;
              current_x_mb_accumulator += dx_mid_bot;

              ushort scan_x1 = x_mb;
              ushort scan_x2 = x_tb;
              if (x_tb < x_mb) {
                  scan_x1 = x_tb;
                  scan_x2 = x_mb;
              }

              if (scan_x1 < scan_x2) {
                  ConnectPoints((Context*)canvas, buffer_idx, value, scan_x1, current_y, scan_x2 - 1, current_y);
              }
          }
      }
    }
  }
}

// Function: PaintRectangle
void PaintRectangle(Canvas* canvas, byte buffer_idx, undefined value, char fill_flag, ushort* rect_params) {
  // rect_params: {x, y, width, height}
  ushort x = rect_params[0];
  ushort y = rect_params[1];
  ushort w = rect_params[2];
  ushort h = rect_params[3];

  if ((x < canvas->width && y < canvas->height) &&
      (x + w <= canvas->width) &&
      (y + h <= canvas->height) &&
      (w != 0 || h != 0)) {

    // Outline
    ConnectPoints((Context*)canvas, buffer_idx, value, x, y, x, y + h -1);
    ConnectPoints((Context*)canvas, buffer_idx, value, x, y + h -1, x + w -1, y + h -1);
    ConnectPoints((Context*)canvas, buffer_idx, value, x + w -1, y, x + w -1, y + h -1);
    ConnectPoints((Context*)canvas, buffer_idx, value, x, y, x + w -1, y);

    if ((fill_flag == 1) && (w > 1)) {
      for (ushort current_y = 1; current_y < h; ++current_y) {
        ConnectPoints((Context*)canvas, buffer_idx, value, x + 1, current_y + y,
                      x + w - 2, current_y + y);
      }
    }
  }
}

// Function: PaintSquare
void PaintSquare(Canvas* canvas, byte buffer_idx, undefined value, char fill_flag, ushort* square_params) {
  // square_params: {x, y, size}
  ushort x = square_params[0];
  ushort y = square_params[1];
  ushort size = square_params[2];

  if ((x < canvas->width && y < canvas->height) &&
      (x + size <= canvas->width) &&
      (y + size <= canvas->height) &&
      (size != 0)) {

    // Outline
    ConnectPoints((Context*)canvas, buffer_idx, value, x, y, x, y + size -1);
    ConnectPoints((Context*)canvas, buffer_idx, value, x, y + size -1, x + size -1, y + size -1);
    ConnectPoints((Context*)canvas, buffer_idx, value, x + size -1, y, x + size -1, y + size -1);
    ConnectPoints((Context*)canvas, buffer_idx, value, x, y, x + size -1, y);

    if ((fill_flag == 1) && (size > 1)) {
      for (ushort current_y = 1; current_y < size; ++current_y) {
        ConnectPoints((Context*)canvas, buffer_idx, value, x + 1, current_y + y,
                      x + size - 2, current_y + y);
      }
    }
  }
}

// Function: PaintLine
void PaintLine(Canvas* canvas, byte buffer_idx, undefined value, byte unused_param, ushort* line_params) {
  // line_params: {x1, y1, x2, y2}
  ushort x1 = line_params[0];
  ushort y1 = line_params[1];
  ushort x2 = line_params[2];
  ushort y2 = line_params[3];

  if ((x1 < canvas->width && y1 < canvas->height) &&
      (x2 < canvas->width && y2 < canvas->height)) {
    ConnectPoints((Context*)canvas, buffer_idx, value, x1, y1, x2, y2);
  }
}

// Function: PaintCircle
void PaintCircle(Canvas* canvas, byte buffer_idx, undefined value, char fill_flag, ushort* circle_params) {
  // circle_params: {x_center, y_center, radius}
  ushort cx = circle_params[0];
  ushort cy = circle_params[1];
  ushort r = circle_params[2];

  // Check if circle is within canvas bounds
  if ((cx >= r && cy >= r) &&
      (cx + r < canvas->width && cy + r < canvas->height)) {

    int current_x = 0;
    ushort current_y = r;
    int p = 1 - r; // Decision parameter for Bresenham's circle algorithm
    int dx = 1;
    int dy = -2 * r;

    if (fill_flag == 0) { // Outline
      SetColor((Context*)canvas, cy + r, cx, buffer_idx, value);
      SetColor((Context*)canvas, cy - r, cx, buffer_idx, value);
      SetColor((Context*)canvas, cy, cx + r, buffer_idx, value);
      SetColor((Context*)canvas, cy, cx - r, buffer_idx, value);
    } else { // Fill (Draw initial vertical line at center)
      ConnectPoints((Context*)canvas, buffer_idx, value, cx, cy - r, cx, cy + r - 1);
    }

    while (current_x < current_y) {
      if (p >= 0) {
        current_y--;
        dy += 2;
        p += dy;
      }
      current_x++;
      dx += 2;
      p += dx;

      if (fill_flag == 0) { // Outline
        SetColor((Context*)canvas, cy + current_y, cx + current_x, buffer_idx, value);
        SetColor((Context*)canvas, cy + current_y, cx - current_x, buffer_idx, value);
        SetColor((Context*)canvas, cy - current_y, cx + current_x, buffer_idx, value);
        SetColor((Context*)canvas, cy - current_y, cx - current_x, buffer_idx, value);
        SetColor((Context*)canvas, cy + current_x, cx + current_y, buffer_idx, value);
        SetColor((Context*)canvas, cy + current_x, cx - current_y, buffer_idx, value);
        SetColor((Context*)canvas, cy - current_x, cx + current_y, buffer_idx, value);
        SetColor((Context*)canvas, cy - current_x, cx - current_y, buffer_idx, value);
      } else { // Fill (Draw horizontal lines for symmetry)
        if (current_y > 0) {
          ConnectPoints((Context*)canvas, buffer_idx, value, cx - current_x, cy - current_y, cx + current_x, cy - current_y);
          ConnectPoints((Context*)canvas, buffer_idx, value, cx - current_x, cy + current_y, cx + current_x, cy + current_y);
        }
        if (current_x > 0) {
          ConnectPoints((Context*)canvas, buffer_idx, value, cx - current_y, cy - current_x, cx + current_y, cy - current_x);
          ConnectPoints((Context*)canvas, buffer_idx, value, cx - current_y, cy + current_x, cx + current_y, cy + current_x);
        }
      }
    }
  }
}

// Function: PaintSpray
void PaintSpray(Canvas* canvas, byte buffer_idx, undefined value, byte unused_param, ushort* spray_params) {
  // spray_params: {x_center, y_center, radius, density, magic_word_low, magic_word_high}
  ushort cx = spray_params[0];
  ushort cy = spray_params[1];
  ushort r = spray_params[2];
  ushort density = spray_params[3];

  // `param_5 + 4` accesses `spray_params[4]`. `*(uint32_t*)(spray_params + 4)` reads `spray_params[4]` and `spray_params[5]` as a single 32-bit int.
  uint32_t magic_word = *(uint32_t*)(spray_params + 4);

  if ((cx < canvas->width && cy < canvas->height) &&
      (magic_word == 0x59745974) && (density < 0x65 && density != 0)) {

    int step_size = 100 / density;
    if (step_size == 0) return;

    // Loop 1: Horizontal lines from center_x outwards
    for (ushort current_x = cx; current_x < cx + r && current_x < canvas->width; current_x += step_size) {
      SetColor((Context*)canvas, cy, current_x, buffer_idx, value);
    }
    for (ushort current_x = cx; (current_x > cx - r || current_x == (ushort)-1) && current_x != (ushort)-1; current_x -= step_size) {
      SetColor((Context*)canvas, cy, current_x, buffer_idx, value);
    }

    // Loop 2: Vertical lines from center_y outwards
    for (ushort current_y = cy; current_y < cy + r && current_y < canvas->height; current_y += step_size) {
      SetColor((Context*)canvas, current_y, cx, buffer_idx, value);
    }
    for (ushort current_y = cy; (current_y > cy - r || current_y == (ushort)-1) && current_y != (ushort)-1; current_y -= step_size) {
      SetColor((Context*)canvas, current_y, cx, buffer_idx, value);
    }

    // Additional points (corners or specific offsets)
    SetColor((Context*)canvas, cy, cx + r, buffer_idx, value);
    SetColor((Context*)canvas, cy, cx - r, buffer_idx, value);
    SetColor((Context*)canvas, cy + r, cx, buffer_idx, value);
    SetColor((Context*)canvas, cy - r, cx, buffer_idx, value);
  }
}


// Main function for demonstration
int main() {
    const ushort SCREEN_WIDTH = 80;
    const ushort SCREEN_HEIGHT = 25;
    const byte PIXEL_BUFFER_IDX = 0; // Assuming we use the first buffer
    const undefined DRAW_COLOR = '@'; // ASCII character to draw

    // Setup Canvas
    Canvas screen_canvas = { .width = SCREEN_WIDTH, .height = SCREEN_HEIGHT };

    // Setup Context
    Context screen_context = { .pitch = SCREEN_WIDTH }; // Pitch is bytes per row
    // Allocate memory for the pixel buffer
    screen_context.buffers[PIXEL_BUFFER_IDX] = (uint8_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint8_t));
    if (screen_context.buffers[PIXEL_BUFFER_IDX] == NULL) {
        fprintf(stderr, "Failed to allocate pixel buffer.\n");
        return 1;
    }
    // Initialize buffer with spaces
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        screen_context.buffers[PIXEL_BUFFER_IDX][i] = ' ';
    }

    // Example drawing operations
    ushort line_params[] = {10, 5, 70, 20}; // x1, y1, x2, y2
    PaintLine(&screen_canvas, PIXEL_BUFFER_IDX, DRAW_COLOR, 0, line_params);

    ushort rect_params[] = {5, 2, 20, 10}; // x, y, width, height
    PaintRectangle(&screen_canvas, PIXEL_BUFFER_IDX, '#', 0, rect_params); // Outline rectangle

    ushort filled_rect_params[] = {30, 15, 15, 8}; // x, y, width, height
    PaintRectangle(&screen_canvas, PIXEL_BUFFER_IDX, 'X', 1, filled_rect_params); // Filled rectangle

    ushort square_params[] = {60, 5, 10}; // x, y, size
    PaintSquare(&screen_canvas, PIXEL_BUFFER_IDX, 'S', 0, square_params); // Outline square

    ushort filled_square_params[] = {40, 0, 5}; // x, y, size
    PaintSquare(&screen_canvas, PIXEL_BUFFER_IDX, 'F', 1, filled_square_params); // Filled square

    ushort circle_params[] = {15, 15, 8}; // x_center, y_center, radius
    PaintCircle(&screen_canvas, PIXEL_BUFFER_IDX, 'O', 0, circle_params); // Outline circle

    ushort filled_circle_params[] = {50, 10, 5}; // x_center, y_center, radius
    PaintCircle(&screen_canvas, PIXEL_BUFFER_IDX, '.', 1, filled_circle_params); // Filled circle

    ushort triangle_params[] = {10, 10, 20, 20, 5, 20}; // x1, y1, x2, y2, x3, y3
    PaintTriangle(&screen_canvas, PIXEL_BUFFER_IDX, 'T', 0, triangle_params); // Outline triangle

    ushort filled_triangle_params[] = {70, 5, 75, 15, 65, 15}; // x1, y1, x2, y2, x3, y3
    PaintTriangle(&screen_canvas, PIXEL_BUFFER_IDX, '*', 1, filled_triangle_params); // Filled triangle

    // Spray params: {x_center, y_center, radius, density, magic_word_low, magic_word_high}
    ushort spray_data[] = {25, 20, 7, 5, 0x5974, 0x5974};
    PaintSpray(&screen_canvas, PIXEL_BUFFER_IDX, '~', 0, spray_data);


    // Print the buffer to console
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            putchar(screen_context.buffers[PIXEL_BUFFER_IDX][y * SCREEN_WIDTH + x]);
        }
        putchar('\n');
    }

    // Clean up
    free(screen_context.buffers[PIXEL_BUFFER_IDX]);
    return 0;
}