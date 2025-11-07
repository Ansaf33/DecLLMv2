#include <stdint.h> // For uint8_t, uint16_t, uint32_t, int32_t, uintptr_t

// Custom types mapping to standard integer types
typedef uint16_t ushort;
typedef uint8_t byte;
typedef uint8_t undefined;
typedef uint32_t undefined4; // Assuming 4-byte undefined value

// Forward declarations for functions
// Assuming fb_context_addr is a uintptr_t representing a base memory address
// and that pointer arithmetic and dereferencing are handled by casting.
int GetColor(uintptr_t fb_context_addr, ushort y, ushort x, byte color_idx_offset);
undefined GetColorIndex(uintptr_t fb_context_addr, ushort y, ushort x, byte color_idx_offset);
void SetColor(uintptr_t fb_context_addr, ushort y, ushort x, byte color_idx_offset, undefined color_val);
void ConnectPoints(uintptr_t fb_context_addr, byte color_idx_offset, undefined color_val, ushort x1, ushort y1, ushort x2, ushort y2);

// Function: GetColor
int GetColor(uintptr_t fb_context_addr, ushort y, ushort x, byte color_idx_offset) {
  uint8_t *base_ptr = (uint8_t*)fb_context_addr;
  // Accessing int32_t at offset 8 + color_idx_offset * 4 (likely an array of scanline pointers)
  int32_t scanline_base_addr = *(int32_t*)(base_ptr + 8 + (uint32_t)color_idx_offset * 4);
  // Accessing ushort at offset 2 (likely screen pitch/width)
  ushort pitch = *(ushort*)(base_ptr + 2);
  // Accessing byte at calculated pixel address
  uint8_t pixel_value = *(uint8_t*)((uint8_t*)scanline_base_addr + (uint32_t)y * pitch + x);
  // Returns fb_context_addr (as int) + pixel_value * 3 + 0x20. This seems to be a palette lookup/color calculation.
  return (int)fb_context_addr + (uint32_t)pixel_value * 3 + 0x20;
}

// Function: GetColorIndex
undefined GetColorIndex(uintptr_t fb_context_addr, ushort y, ushort x, byte color_idx_offset) {
  uint8_t *base_ptr = (uint8_t*)fb_context_addr;
  int32_t scanline_base_addr = *(int32_t*)(base_ptr + 8 + (uint32_t)color_idx_offset * 4);
  ushort pitch = *(ushort*)(base_ptr + 2);
  return *(uint8_t*)((uint8_t*)scanline_base_addr + (uint32_t)y * pitch + x);
}

// Function: SetColor
void SetColor(uintptr_t fb_context_addr, ushort y, ushort x, byte color_idx_offset, undefined color_val) {
  uint8_t *base_ptr = (uint8_t*)fb_context_addr;
  int32_t scanline_base_addr = *(int32_t*)(base_ptr + 8 + (uint32_t)color_idx_offset * 4);
  if (scanline_base_addr != 0) {
    ushort pitch = *(ushort*)(base_ptr + 2);
    *(uint8_t*)((uint8_t*)scanline_base_addr + (uint32_t)y * pitch + x) = color_val;
  }
}

// Function: PaintTriangle
void PaintTriangle(uintptr_t fb_context_addr, byte color_idx_offset, undefined color_val, char fill_flag, ushort *vertices) {
  // Assuming fb_context_addr points to a struct where width is at offset 0 and height at offset 2.
  ushort screen_width = *(ushort*)((uint8_t*)fb_context_addr + 0);
  ushort screen_height = *(ushort*)((uint8_t*)fb_context_addr + 2);

  // vertices: [x1, y1, x2, y2, x3, y3]
  ushort v0_x = vertices[0], v0_y = vertices[1];
  ushort v1_x = vertices[2], v1_y = vertices[3];
  ushort v2_x = vertices[4], v2_y = vertices[5];

  // Boundary check
  if (!((v0_x < screen_height && v1_x < screen_height && v2_x < screen_height) &&
        (v0_y < screen_width && v1_y < screen_width && v2_y < screen_width))) {
    return;
  }

  if (fill_flag == 0) { // Outline
    ConnectPoints(fb_context_addr, color_idx_offset, color_val, v0_x, v0_y, v1_x, v1_y);
    ConnectPoints(fb_context_addr, color_idx_offset, color_val, v2_x, v2_y, v0_x, v0_y);
    ConnectPoints(fb_context_addr, color_idx_offset, color_val, v1_x, v1_y, v2_x, v2_y);
  } else { // Filled
    struct Point { ushort x, y; };
    struct Point p[3] = {{v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}};

    // Sort points by y-coordinate (p[0] = top, p[1] = mid, p[2] = bot)
    for (int i = 0; i < 2; ++i) {
      for (int j = i + 1; j < 3; ++j) {
        if (p[j].y < p[i].y) {
          struct Point temp = p[i];
          p[i] = p[j];
          p[j] = temp;
        }
      }
    }

    ushort x_top = p[0].x, y_top = p[0].y;
    ushort x_mid = p[1].x, y_mid = p[1].y;
    ushort x_bot = p[2].x, y_bot = p[2].y;

    // Handle flat-bottom triangle case (y_mid == y_bot)
    if (y_bot == y_mid) {
      ushort min_x = x_top, max_x = x_top;
      if (x_mid < min_x) min_x = x_mid; else if (x_mid > max_x) max_x = x_mid;
      if (x_bot < min_x) min_x = x_bot; else if (x_bot > max_x) max_x = x_bot;
      if (min_x < max_x) {
        ConnectPoints(fb_context_addr, color_idx_offset, color_val, min_x, y_mid, max_x - 1, y_mid);
      }
    } else { // General triangle fill
      int acc_long_edge_dx = 0; // Accumulator for the edge from top to bottom (p[0] to p[2])
      int acc_current_edge_dx = 0; // Accumulator for the current shorter edge (p[0] to p[1] or p[1] to p[2])

      // First half: from y_top to y_mid
      if (y_top == y_mid) { // Flat-top triangle
        ushort min_x_flat_top = x_top;
        ushort max_x_flat_top = x_mid;
        if (min_x_flat_top > max_x_flat_top) {
            ushort temp = min_x_flat_top; min_x_flat_top = max_x_flat_top; max_x_flat_top = temp;
        }
        if (min_x_flat_top < max_x_flat_top) {
            ConnectPoints(fb_context_addr, color_idx_offset, color_val, min_x_flat_top, y_top, max_x_flat_top - 1, y_top);
        }
        // Initialize accumulators for the second half, starting from y_mid
        acc_long_edge_dx = (int)(x_bot - x_top) * (y_mid - y_top);
        acc_current_edge_dx = (int)(x_bot - x_mid) * (y_mid - y_mid); // Will be 0
      } else { // Standard first half
        for (ushort y_scan = y_top; y_scan <= y_mid; ++y_scan) {
          int delta_y_long = y_bot - y_top;
          int delta_y_current = y_mid - y_top;

          ushort current_x_long = x_top;
          if (delta_y_long != 0) {
            current_x_long = x_top + acc_long_edge_dx / delta_y_long;
          }

          ushort current_x_current = x_top;
          if (delta_y_current != 0) {
            current_x_current = x_top + acc_current_edge_dx / delta_y_current;
          }

          acc_long_edge_dx += (x_bot - x_top);
          acc_current_edge_dx += (x_mid - x_top);

          ushort fill_x_start = current_x_current;
          ushort fill_x_end = current_x_long;
          if (current_x_long < current_x_current) {
            fill_x_start = current_x_long;
            fill_x_end = current_x_current;
          }
          if (fill_x_start < fill_x_end) {
            ConnectPoints(fb_context_addr, color_idx_offset, color_val, fill_x_start, y_scan, (fill_x_end - 1), y_scan);
          }
        }
      }

      // Second half: from y_mid to y_bot
      // Re-initialize accumulators based on the state at y_mid
      acc_long_edge_dx = (int)(x_bot - x_top) * (y_mid - y_top); // Accumulator for long edge at y_mid
      acc_current_edge_dx = (int)(x_bot - x_mid) * (y_mid - y_mid); // Accumulator for new short edge (p[1] to p[2]) at y_mid

      for (ushort y_scan = y_mid; y_scan <= y_bot; ++y_scan) {
        int delta_y_long = y_bot - y_top;
        int delta_y_current = y_bot - y_mid;

        ushort current_x_long = x_top;
        if (delta_y_long != 0) {
          current_x_long = x_top + acc_long_edge_dx / delta_y_long;
        }

        ushort current_x_current = x_mid;
        if (delta_y_current != 0) {
          current_x_current = x_mid + acc_current_edge_dx / delta_y_current;
        }

        acc_long_edge_dx += (x_bot - x_top);
        acc_current_edge_dx += (x_bot - x_mid);

        ushort fill_x_start = current_x_current;
        ushort fill_x_end = current_x_long;
        if (current_x_long < current_x_current) {
          fill_x_start = current_x_long;
          fill_x_end = current_x_current;
        }
        if (fill_x_start < fill_x_end) {
          ConnectPoints(fb_context_addr, color_idx_offset, color_val, fill_x_start, y_scan, (fill_x_end - 1), y_scan);
        }
      }
    }
  }
}

// Function: PaintRectangle
void PaintRectangle(uintptr_t fb_context_addr, byte color_idx_offset, undefined color_val, char fill_flag, ushort *rect_params) {
  // rect_params: [x, y, width, height]
  ushort x = rect_params[0], y = rect_params[1];
  ushort width = rect_params[2], height = rect_params[3];

  ushort screen_width = *(ushort*)((uint8_t*)fb_context_addr + 0);
  ushort screen_height = *(ushort*)((uint8_t*)fb_context_addr + 2);

  // Boundary check
  if (!((x < screen_height && y < screen_width) &&
        ((uint32_t)x + width < screen_height) &&
        ((uint32_t)y + height < screen_width) &&
        (width != 0 || height != 0))) {
    return;
  }

  // Draw outline
  ConnectPoints(fb_context_addr, color_idx_offset, color_val, x, y, x, y + height);
  ConnectPoints(fb_context_addr, color_idx_offset, color_val, x, y + height, x + width, y + height);
  ConnectPoints(fb_context_addr, color_idx_offset, color_val, x + width, y, x + width, y + height);
  ConnectPoints(fb_context_addr, color_idx_offset, color_val, x, y, x + width, y);

  if (fill_flag == 1 && width > 1) {
    for (ushort row = 1; row < height; ++row) {
      ConnectPoints(fb_context_addr, color_idx_offset, color_val, x + 1, row + y, x + width - 1, row + y);
    }
  }
}

// Function: PaintSquare
void PaintSquare(uintptr_t fb_context_addr, byte color_idx_offset, undefined color_val, char fill_flag, ushort *square_params) {
  // square_params: [x, y, size]
  ushort x = square_params[0], y = square_params[1];
  ushort size = square_params[2];

  ushort screen_width = *(ushort*)((uint8_t*)fb_context_addr + 0);
  ushort screen_height = *(ushort*)((uint8_t*)fb_context_addr + 2);

  // Boundary check
  if (!((x < screen_height && y < screen_width) &&
        ((uint32_t)x + size < screen_height) &&
        ((uint32_t)y + size < screen_width) &&
        (size != 0))) {
    return;
  }

  // Draw outline
  ConnectPoints(fb_context_addr, color_idx_offset, color_val, x, y, x, y + size);
  ConnectPoints(fb_context_addr, color_idx_offset, color_val, x, y + size, x + size, y + size);
  ConnectPoints(fb_context_addr, color_idx_offset, color_val, x + size, y, x + size, y + size);
  ConnectPoints(fb_context_addr, color_idx_offset, color_val, x, y, x + size, y);

  if (fill_flag == 1 && size > 1) {
    for (ushort row = 1; row < size; ++row) {
      ConnectPoints(fb_context_addr, color_idx_offset, color_val, x + 1, row + y, x + size - 1, row + y);
    }
  }
}

// Function: ConnectPoints (Bresenham-like line drawing)
void ConnectPoints(uintptr_t fb_context_addr, byte color_idx_offset, undefined color_val, ushort x1, ushort y1, ushort x2, ushort y2) {
  int dx_abs, dy_abs;
  short sx, sy; // Step direction
  int max_dim; // Max of dx_abs or dy_abs
  int error_x_acc, error_y_acc; // Accumulators for error terms

  // Calculate absolute differences and step directions
  dx_abs = (int)x2 > (int)x1 ? (int)x2 - (int)x1 : (int)x1 - (int)x2;
  sx = x1 < x2 ? 1 : -1;

  dy_abs = (int)y2 > (int)y1 ? (int)y2 - (int)y1 : (int)y1 - (int)y2;
  sy = y1 < y2 ? 1 : -1;

  // Determine the number of steps (max dimension)
  max_dim = dx_abs > dy_abs ? dx_abs : dy_abs;

  error_x_acc = 0;
  error_y_acc = 0;

  // Loop `max_dim + 2` times as per original code, which draws `max_dim + 2` points.
  for (int i = 0; i <= max_dim + 1; ++i) {
    SetColor(fb_context_addr, y1, x1, color_idx_offset, color_val);
    error_x_acc += dx_abs;
    error_y_acc += dy_abs;

    // Adjust x if accumulated error exceeds max_dim
    if (max_dim < error_x_acc) {
      x1 += sx;
      error_x_acc -= max_dim;
    }
    // Adjust y if accumulated error exceeds max_dim
    if (max_dim < error_y_acc) {
      y1 += sy;
      error_y_acc -= max_dim;
    }
  }
}

// Function: PaintLine
void PaintLine(uintptr_t fb_context_addr, byte color_idx_offset, undefined color_val, ushort *line_params) {
  // line_params: [x1, y1, x2, y2]
  ushort x1 = line_params[0], y1 = line_params[1];
  ushort x2 = line_params[2], y2 = line_params[3];

  ushort screen_width = *(ushort*)((uint8_t*)fb_context_addr + 0);
  ushort screen_height = *(ushort*)((uint8_t*)fb_context_addr + 2);

  // Boundary check
  if (!((x1 < screen_height && x1 <= x2) && (y1 < screen_width) &&
        (y1 <= y2 && y2 < screen_width) && (x2 < screen_height))) {
    return;
  }

  ConnectPoints(fb_context_addr, color_idx_offset, color_val, x1, y1, x2, y2);
}

// Function: PaintCircle
void PaintCircle(uintptr_t fb_context_addr, byte color_idx_offset, undefined color_val, char fill_flag, ushort *circle_params) {
  // circle_params: [x_center, y_center, radius]
  ushort xc = circle_params[0], yc = circle_params[1];
  ushort r = circle_params[2];

  ushort screen_width = *(ushort*)((uint8_t*)fb_context_addr + 0);
  ushort screen_height = *(uint8_t*)((uint8_t*)fb_context_addr + 2);

  // Boundary check: circle must be within screen bounds
  if (!((xc < screen_height && yc < screen_width) &&
        ((uint32_t)xc + r < screen_height) &&
        ((uint32_t)yc + r < screen_width) &&
        ((int32_t)xc - r >= -1) && // check against -1 due to original comparison
        ((int32_t)yc - r >= -1))) { // check against -1 due to original comparison
    return;
  }

  int x_current = 0;
  int y_current = r;
  int decision_param = 1 - r; // Corresponds to local_1c
  int d_east_delta = 1;       // Corresponds to local_20
  int d_north_east_delta = (int)r * -2; // Corresponds to local_24

  if (fill_flag == 0) { // Outline
    SetColor(fb_context_addr, yc + r, xc, color_idx_offset, color_val);
    SetColor(fb_context_addr, yc - r, xc, color_idx_offset, color_val);
    SetColor(fb_context_addr, yc, xc + r, color_idx_offset, color_val);
    SetColor(fb_context_addr, yc, xc - r, color_idx_offset, color_val);
  } else { // Filled
    ConnectPoints(fb_context_addr, color_idx_offset, color_val, xc, yc - r, xc, (yc + r) - 1);
  }

  while (x_current < y_current) {
    if (-1 < decision_param) { // If decision_param >= 0 (original `-1 < local_1c`)
      y_current--;
      d_north_east_delta += 2;
      decision_param += d_north_east_delta;
    }
    x_current++;
    d_east_delta += 2;
    decision_param += d_east_delta;

    if (fill_flag == 0) { // Outline
      SetColor(fb_context_addr, yc + y_current, xc + x_current, color_idx_offset, color_val);
      SetColor(fb_context_addr, yc + y_current, xc - x_current, color_idx_offset, color_val);
      SetColor(fb_context_addr, yc - y_current, xc + x_current, color_idx_offset, color_val);
      SetColor(fb_context_addr, yc - y_current, xc - x_current, color_idx_offset, color_val);
      SetColor(fb_context_addr, yc + x_current, xc + y_current, color_idx_offset, color_val);
      SetColor(fb_context_addr, yc + x_current, xc - y_current, color_idx_offset, color_val);
      SetColor(fb_context_addr, yc - x_current, xc + y_current, color_idx_offset, color_val);
      SetColor(fb_context_addr, yc - x_current, xc - y_current, color_idx_offset, color_val);
    } else { // Filled
      if (y_current > 0) {
        ConnectPoints(fb_context_addr, color_idx_offset, color_val, xc + x_current, yc - y_current, xc + x_current, (yc + y_current) - 1);
        ConnectPoints(fb_context_addr, color_idx_offset, color_val, xc - x_current, yc - y_current, xc - x_current, (yc + y_current) - 1);
      }
      if (x_current > 0) {
        ConnectPoints(fb_context_addr, color_idx_offset, color_val, xc + y_current, yc - x_current, xc + y_current, (yc + x_current) - 1);
        ConnectPoints(fb_context_addr, color_idx_offset, color_val, xc - y_current, yc - x_current, xc - y_current, (yc + x_current) - 1);
      }
    }
  }
}

// Function: PaintSpray
void PaintSpray(uintptr_t fb_context_addr, byte color_idx_offset, undefined color_val, ushort *spray_params) {
  // spray_params: [x_center, y_center, radius, density, magic_val_low, magic_val_high]
  // Note: magic_val is 0x59745974
  ushort xc = spray_params[0];
  ushort yc = spray_params[1];
  ushort radius = spray_params[2];
  ushort density_divisor = spray_params[3];

  uint32_t magic_val = *(uint32_t*)&spray_params[4]; // *(int *)(param_5 + 4)

  ushort screen_width = *(ushort*)((uint8_t*)fb_context_addr + 0);
  ushort screen_height = *(ushort*)((uint8_t*)fb_context_addr + 2);

  // Boundary and parameter checks
  if (!((xc < screen_height && yc < screen_width) &&
        (magic_val == 0x59745974) &&
        (density_divisor < 0x65 && density_divisor != 0))) {
    return;
  }

  int step_size = 100 / (int)density_divisor;
  if (step_size == 0) return; // Avoid division by zero or infinite loop if step_size is 0.

  // Draw horizontal lines (along y_center)
  for (ushort current_x = xc; (int)current_x < (int)(radius + xc) && (int)current_x < (int)screen_height; current_x += step_size) {
    SetColor(fb_context_addr, yc, current_x, color_idx_offset, color_val);
  }
  for (ushort current_x = xc; (int)(xc - radius) < (int)current_x && (int)current_x >= 0; current_x -= step_size) {
    SetColor(fb_context_addr, yc, current_x, color_idx_offset, color_val);
  }

  // Draw vertical lines (along x_center)
  for (ushort current_y = yc; (int)current_y < (int)(radius + yc) && (int)current_y < (int)screen_width; current_y += step_size) {
    SetColor(fb_context_addr, current_y, xc, color_idx_offset, color_val);
  }
  for (ushort current_y = yc; (int)(yc - radius) < (int)current_y && (int)current_y >= 0; current_y -= step_size) {
    SetColor(fb_context_addr, current_y, xc, color_idx_offset, color_val);
  }

  // Draw four cardinal points at radius distance
  SetColor(fb_context_addr, yc, xc + radius, color_idx_offset, color_val);
  SetColor(fb_context_addr, yc, xc - radius, color_idx_offset, color_val);
  SetColor(fb_context_addr, yc + radius, xc, color_idx_offset, color_val);
  SetColor(fb_context_addr, yc - radius, xc, color_idx_offset, color_val);
}