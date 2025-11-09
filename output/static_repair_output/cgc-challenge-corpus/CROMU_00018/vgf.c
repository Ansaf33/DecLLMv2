#include <stdint.h>  // For fixed-width integer types like uint8_t, uint16_t, uint32_t
#include <stdlib.h>  // For malloc, free, abs
#include <string.h>  // For memcpy, memset
#include <stdbool.h> // For bool type

// Standardized types based on common usage in decompiled code
typedef uint8_t  byte;
typedef uint16_t ushort;
typedef uint32_t uint;

// Structure for the file header (11 bytes)
typedef struct VGFHeader {
    uint32_t magic;    // 0x00
    uint16_t version;  // 0x04
    uint16_t height;   // 0x06
    uint16_t width;    // 0x08
    uint8_t  depth;    // 0x0A
} VGFHeader;

// Structure for an element (rectangle, circle, line, triangle)
// Allocated with malloc(0x14) = 20 bytes.
// The first uint32_t (type_and_data_0) holds the type in its lowest byte,
// followed by layer_idx, color_idx, and fill_flag in subsequent bytes (assuming little-endian or direct byte access).
// The remaining data_1, data_2, data_3 are interpreted as ushort coordinates by rendering functions.
typedef struct VGFElement {
    uint32_t type_and_data_0; // 0x00: Type (byte 0), Layer (byte 1), Color (byte 2), Fill_flag (byte 3)
    uint32_t data_1;          // 0x04
    uint32_t data_2;          // 0x08
    uint32_t data_3;          // 0x0C
    struct VGFElement *next;  // 0x10 (pointer to next element in list)
} VGFElement; // Size: 20 bytes (0x14)

// Main file structure (allocated with malloc(0x18) = 24 bytes)
typedef struct VGFFile {
    VGFHeader header;           // 0x00 - 0x0A (11 bytes)
    VGFElement *elements;       // 0x0C (pointer to first element)
    uint8_t palette_size;       // 0x10
    uint8_t *palette_data;      // 0x14 (pointer to palette data, RGB triplets)
} VGFFile; // Size: 24 bytes (0x18)

// --- Function Prototypes ---
void vgf_destroy_file(void *file_ptr);
void vgf_hline_helper(ushort x, ushort y, ushort length, int layer_ptr_int, byte color_idx, ushort width, ushort height);
void vgf_vline_helper(ushort x, ushort y, ushort length, int layer_ptr_int, byte color_idx, ushort width, ushort height);
void vgf_line_helper(ushort x0, ushort y0, ushort x1, ushort y1, int layer_ptr_int, byte color_idx, ushort width, ushort height);
void vgf_render_rect(ushort *coords, int layer_ptr_int, byte fill_flag, byte color_idx, ushort width, ushort height);
void vgf_render_circle(ushort *coords, int layer_ptr_int, uint fill_flag, byte color_idx, ushort width, ushort height);
void vgf_render_triangle(ushort *coords, int layer_ptr_int, uint fill_flag, byte color_idx, ushort width, ushort height);
void vgf_render_line(ushort *coords, int layer_ptr_int, uint fill_flag, byte color_idx, ushort width, ushort height);


// Function: vgf_get_width
ushort vgf_get_width(const VGFFile *file) {
  return (file == NULL) ? 0 : file->header.width;
}

// Function: vgf_get_height
ushort vgf_get_height(const VGFFile *file) {
  return (file == NULL) ? 0 : file->header.height;
}

// Function: vgf_parse_data
int vgf_parse_data(const int *data_ptr, uint data_size, void **out_file_ptr) {
    VGFFile *file = NULL;
    int result = -1; // Default to error
    VGFElement *last_element = NULL;
    uint current_offset = 0; // Tracks current position in input data_ptr

    *out_file_ptr = NULL;

    // 1. Initial header checks
    if (data_size < sizeof(VGFHeader)) {
        return -1;
    }

    const VGFHeader *header_in = (const VGFHeader *)data_ptr;

    if (header_in->magic != 0x78330909) {
        return -1;
    }
    if (header_in->version != 1) {
        return -2; // Specific error code for version mismatch
    }
    if (header_in->height >= 0x201 || header_in->width >= 0x201) { // 513
        return -1;
    }
    if (header_in->depth >= 7) { // Max 6 layers
        return -1;
    }

    // 2. Allocate VGFFile and copy header
    file = (VGFFile *)malloc(sizeof(VGFFile));
    if (file == NULL) {
        return -1;
    }
    memset(file, 0, sizeof(VGFFile)); // Initialize all fields to 0
    memcpy(&file->header, header_in, sizeof(VGFHeader));
    current_offset = sizeof(VGFHeader); // 11 bytes

    // 3. Element parsing loop
    bool elements_end_reached = false;
    while (!elements_end_reached) {
        uint element_total_bytes = 0; // Total bytes for the current element
        byte element_type;

        // Read the first 4 bytes (type_and_data_0) to determine type and data size
        if (data_size < current_offset + sizeof(uint32_t)) {
            result = -1;
            break;
        }
        element_type = *(byte *)((uintptr_t)data_ptr + current_offset);

        if (element_type == 100) { // End of elements marker
            elements_end_reached = true;
            element_total_bytes = sizeof(uint32_t); // Consume the 4 bytes of the marker
        } else if (element_type > 3) { // Invalid type (0,1,2,3 valid)
            result = -1;
            break;
        } else { // Types 0, 1, 2, 3
            switch (element_type) {
                case 3: // Circle: 4 (type_data_0) + 4 (data_1) + 2 (data_2_ushort) = 10 bytes
                    element_total_bytes = 10;
                    break;
                case 2: // Line: 4 (type_data_0) + 4 (data_1) + 4 (data_2) = 12 bytes
                case 0: // Rect: 4 (type_data_0) + 4 (data_1) + 4 (data_2) = 12 bytes
                    element_total_bytes = 12;
                    break;
                case 1: // Triangle: 4 (type_data_0) + 4 (data_1) + 4 (data_2) + 4 (data_3) = 16 bytes
                    element_total_bytes = 16;
                    break;
            }

            if (data_size < current_offset + element_total_bytes) {
                result = -1;
                break;
            }

            VGFElement *new_element = (VGFElement *)malloc(sizeof(VGFElement));
            if (new_element == NULL) {
                result = -1;
                break;
            }
            memset(new_element, 0, sizeof(VGFElement));

            if (last_element == NULL) {
                file->elements = new_element;
            } else {
                last_element->next = new_element;
            }

            memcpy(new_element, (byte *)((uintptr_t)data_ptr + current_offset), element_total_bytes);
            last_element = new_element;
        }
        current_offset += element_total_bytes; // Advance past current element
    }

    if (result != -1) { // If elements parsing was successful
        // 4. Palette parsing
        if (data_size < current_offset + 1) { // Need at least 1 byte for palette size
            result = -1;
        } else {
            file->palette_size = *(byte *)((uintptr_t)data_ptr + current_offset);
            current_offset += 1;

            if (file->palette_size > 0) {
                uint palette_data_len = file->palette_size * 3; // RGB triplets
                if (data_size < current_offset + palette_data_len) {
                    result = -1;
                } else {
                    file->palette_data = (byte *)malloc(palette_data_len);
                    if (file->palette_data == NULL) {
                        result = -1;
                    } else {
                        memcpy(file->palette_data, (byte *)((uintptr_t)data_ptr + current_offset), palette_data_len);
                        result = 0; // Success
                    }
                }
            } else {
                result = 0; // Success, no palette data
            }
        }
    }

    // 5. Cleanup or assign result
    if (result != 0 && file != NULL) {
        vgf_destroy_file(file);
        file = NULL;
    }
    *out_file_ptr = file;

    return result;
}

// Function: vgf_destroy_file
void vgf_destroy_file(void *file_ptr) {
  VGFFile *file = (VGFFile *)file_ptr;
  if (file != NULL) {
    VGFElement *current_element = file->elements;
    while (current_element != NULL) {
      VGFElement *next_element = current_element->next;
      free(current_element);
      current_element = next_element;
    }
    if (file->palette_data != NULL) {
      free(file->palette_data);
    }
    free(file);
  }
}

// Function: vgf_get_render_size
int vgf_get_render_size(const VGFFile *file) {
  return (file == NULL) ? -1 : (int)file->header.height * (int)file->header.width * 3;
}

// Function: vgf_render_file
int vgf_render_file(const VGFFile *file, uint8_t *output_buffer, uint *output_buffer_size_ptr) {
    if (file == NULL || output_buffer == NULL || output_buffer_size_ptr == NULL) {
        return -1;
    }

    int result = 0; // Assume success
    ushort width = file->header.width;
    ushort height = file->header.height;
    uint8_t depth = file->header.depth;
    uint total_pixels = (uint)width * height;
    
    uint16_t *layers[6] = {NULL}; // Max depth is 6 (from header check < 7)
    uint16_t *render_target_merged = NULL; // Stores final merged layer data (ushort)

    // Initialize layers
    for (uint8_t i = 0; i < depth; ++i) {
        layers[i] = (uint16_t *)malloc(total_pixels * sizeof(uint16_t));
        if (layers[i] == NULL) {
            result = -1;
            break;
        }
        // Initialize layer with 0xffff (transparent/empty pixel)
        for (uint j = 0; j < total_pixels; ++j) {
            layers[i][j] = 0xffff;
        }
    }

    if (result != 0) { // If layer allocation failed
        goto cleanup_and_return;
    }

    // Render elements onto layers
    for (VGFElement *elem = file->elements; elem != NULL; elem = elem->next) {
        byte type = (byte)(elem->type_and_data_0 & 0xFF);
        byte element_layer = (byte)((elem->type_and_data_0 >> 8) & 0xFF);
        byte element_color = (byte)((elem->type_and_data_0 >> 16) & 0xFF);
        byte fill_flag = (byte)((elem->type_and_data_0 >> 24) & 0xFF);

        if (element_layer >= depth) {
            result = -1;
            break;
        }
        if (element_color >= file->palette_size) {
            result = -1;
            break;
        }

        uint16_t *current_layer = layers[element_layer];
        ushort *element_data_ptr = (ushort *)&(elem->data_1); // Data starts after type_and_data_0

        switch (type) {
            case 3: // Circle
                vgf_render_circle(element_data_ptr, (int)current_layer, fill_flag, element_color, width, height);
                break;
            case 2: // Line
                vgf_render_line(element_data_ptr, (int)current_layer, fill_flag, element_color, width, height);
                break;
            case 0: // Rectangle
                vgf_render_rect(element_data_ptr, (int)current_layer, fill_flag, element_color, width, height);
                break;
            case 1: // Triangle
                vgf_render_triangle(element_data_ptr, (int)current_layer, fill_flag, element_color, width, height);
                break;
            default: // Invalid type
                result = -1;
                break;
        }
        if (result != 0) {
            break;
        }
    }

    if (result != 0) {
        goto cleanup_and_return;
    }

    // Merge layers
    render_target_merged = (uint16_t *)malloc(total_pixels * sizeof(uint16_t));
    if (render_target_merged == NULL) {
        result = -1;
        goto cleanup_and_return;
    }
    for (uint j = 0; j < total_pixels; ++j) {
        render_target_merged[j] = 0xffff; // Initialize merged layer to transparent
    }

    for (uint8_t i = 0; i < depth; ++i) {
        for (uint j = 0; j < total_pixels; ++j) {
            if (layers[i][j] != 0xffff) {
                render_target_merged[j] = layers[i][j];
            }
        }
    }

    // Convert merged layer to RGB output buffer
    if (*output_buffer_size_ptr < total_pixels * 3) {
        result = -1;
    } else {
        uint current_output_idx = 0;
        for (uint j = 0; j < total_pixels; ++j) {
            if (render_target_merged[j] == 0xffff) { // Transparent pixel
                output_buffer[current_output_idx++] = 0;
                output_buffer[current_output_idx++] = 0;
                output_buffer[current_output_idx++] = 0;
            } else {
                ushort palette_entry_idx = render_target_merged[j];
                if (palette_entry_idx >= file->palette_size) {
                    result = -1;
                    break;
                }
                output_buffer[current_output_idx++] = file->palette_data[palette_entry_idx * 3];
                output_buffer[current_output_idx++] = file->palette_data[palette_entry_idx * 3 + 1];
                output_buffer[current_output_idx++] = file->palette_data[palette_entry_idx * 3 + 2];
            }
        }
    }

cleanup_and_return:
    if (render_target_merged != NULL) {
        free(render_target_merged);
    }
    for (uint8_t i = 0; i < depth; ++i) {
        if (layers[i] != NULL) {
            free(layers[i]);
        }
    }

    return result;
}

// Function: vgf_hline_helper
void vgf_hline_helper(ushort x, ushort y, ushort length, int layer_ptr_int, byte color_idx, ushort width, ushort height) {
  if (x < width && y < height) {
    for (ushort i = 0; i < length; ++i) {
      if ((uint)x + i < width) {
        *(ushort *)(((uint)width * (uint)y + (uint)x + i) * 2 + layer_ptr_int) = (ushort)color_idx;
      } else {
        break; // Stop if line goes out of bounds
      }
    }
  }
}

// Function: vgf_vline_helper
void vgf_vline_helper(ushort x, ushort y, ushort length, int layer_ptr_int, byte color_idx, ushort width, ushort height) {
  if (x < width && y < height) {
    for (ushort i = 0; i < length; ++i) {
      if ((uint)y + i < height) {
        *(ushort *)(((uint)width * ((uint)y + i) + (uint)x) * 2 + layer_ptr_int) = (ushort)color_idx;
      } else {
        break; // Stop if line goes out of bounds
      }
    }
  }
}

// Function: vgf_line_helper (Bresenham-like algorithm)
void vgf_line_helper(ushort x0, ushort y0, ushort x1, ushort y1, int layer_ptr_int, byte color_idx, ushort width, ushort height) {
  int dx = abs((int)x1 - (int)x0);
  int dy = abs((int)y1 - (int)y0);
  int steps = (dx > dy) ? dx : dy;

  int step_x = (x0 < x1) ? 1 : -1;
  int step_y = (y0 < y1) ? 1 : -1;

  int error_x_accum = 0;
  int error_y_accum = 0;

  for (int i = 0; i <= steps + 1; ++i) {
    if (x0 < width && y0 < height) {
        *(ushort *)(((uint)width * (uint)y0 + (uint)x0) * 2 + layer_ptr_int) = (ushort)color_idx;
    }
    
    error_x_accum += dx;
    error_y_accum += dy;

    if (error_x_accum > steps) {
      error_x_accum -= steps;
      x0 += step_x;
    }
    if (error_y_accum > steps) {
      error_y_accum -= steps;
      y0 += step_y;
    }
  }
}

// Function: vgf_render_rect
void vgf_render_rect(ushort *coords, int layer_ptr_int, byte fill_flag, byte color_idx, ushort width, ushort height) {
  if (coords == NULL) return;

  ushort x0 = coords[0];
  ushort y0 = coords[1];
  ushort rect_width = coords[2];
  ushort rect_height = coords[3];
  
  // Calculate end coordinates
  ushort x1 = x0 + rect_width;
  ushort y1 = y0 + rect_height;

  // Validate coordinates and bounds
  if (x0 >= width || y0 >= height || x1 > width || y1 > height || x0 > x1 || y0 > y1) {
    return;
  }
  
  // Boundary drawing
  ushort current_x;
  ushort current_y;

  // Draw top line (x0 to x1)
  for (current_x = x0; current_x < x1; ++current_x) {
    *(ushort *)(((uint)width * (uint)y0 + (uint)current_x) * 2 + layer_ptr_int) = (ushort)color_idx;
  }
  // Draw right line (y0 to y1)
  for (current_y = y0; current_y < y1; ++current_y) {
    *(ushort *)(((uint)width * (uint)current_y + (uint)x1) * 2 + layer_ptr_int) = (ushort)color_idx;
  }
  // Draw bottom line (x1 down to x0)
  for (current_x = x1; current_x > x0; --current_x) {
    *(ushort *)(((uint)width * (uint)y1 + (uint)current_x) * 2 + layer_ptr_int) = (ushort)color_idx;
  }
  // Draw left line (y1 down to y0)
  for (current_y = y1; current_y > y0; --current_y) {
    *(ushort *)(((uint)width * (uint)current_y + (uint)x0) * 2 + layer_ptr_int) = (ushort)color_idx;
  }

  // Fill logic
  if ((fill_flag & 1) != 0) {
    for (current_x = x0 + 1; current_x < x1; ++current_x) {
      for (current_y = y0 + 1; current_y < y1; ++current_y) {
        *(ushort *)(((uint)width * (uint)current_y + (uint)current_x) * 2 + layer_ptr_int) = (ushort)color_idx;
      }
    }
  }
}

// Function: vgf_render_circle (Bresenham-like algorithm)
void vgf_render_circle(ushort *coords, int layer_ptr_int, uint fill_flag, byte color_idx, ushort width, ushort height) {
  if (coords == NULL) return;

  ushort center_x = coords[0];
  ushort center_y = coords[1];
  ushort radius = coords[2];

  // Check if circle is within bounds (simplified check)
  if (center_x + radius >= width || center_x < radius ||
      center_y + radius >= height || center_y < radius) {
    return;
  }

  if ((fill_flag & 1) == 0) { // Wireframe circle
    int x = 0;
    int y = radius;
    int d = 1 - radius; // Decision parameter
    int delta_e = 3;
    int delta_se = -2 * (int)radius + 5;

    // Plot initial points for 0, R, -R
    if (center_x < width && center_y + radius < height)
        *(ushort *)(((uint)width * (uint)(center_y + radius) + (uint)center_x) * 2 + layer_ptr_int) = (ushort)color_idx;
    if (center_x < width && center_y - radius < height)
        *(ushort *)(((uint)width * (uint)(center_y - radius) + (uint)center_x) * 2 + layer_ptr_int) = (ushort)color_idx;
    if (center_x + radius < width && center_y < height)
        *(ushort *)(((uint)width * (uint)center_y + (uint)(center_x + radius)) * 2 + layer_ptr_int) = (ushort)color_idx;
    if (center_x - radius < width && center_y < height)
        *(ushort *)(((uint)width * (uint)center_y + (uint)(center_x - radius)) * 2 + layer_ptr_int) = (ushort)color_idx;

    while (x < y) {
      if (d < 0) { // Move East
        d += delta_e;
        delta_e += 2;
        delta_se += 2;
      } else { // Move South-East
        d += delta_se;
        delta_e += 2;
        delta_se += 4;
        y--;
      }
      x++;

      // Plot 8 symmetric points
      if (center_x + x < width && center_y + y < height)
          *(ushort *)(((uint)width * (uint)(center_y + y) + (uint)(center_x + x)) * 2 + layer_ptr_int) = (ushort)color_idx;
      if (center_x - x < width && center_y + y < height)
          *(ushort *)(((uint)width * (uint)(center_y + y) + (uint)(center_x - x)) * 2 + layer_ptr_int) = (ushort)color_idx;
      if (center_x + x < width && center_y - y < height)
          *(ushort *)(((uint)width * (uint)(center_y - y) + (uint)(center_x + x)) * 2 + layer_ptr_int) = (ushort)color_idx;
      if (center_x - x < width && center_y - y < height)
          *(ushort *)(((uint)width * (uint)(center_y - y) + (uint)(center_x - x)) * 2 + layer_ptr_int) = (ushort)color_idx;
      if (center_x + y < width && center_y + x < height)
          *(ushort *)(((uint)width * (uint)(center_y + x) + (uint)(center_x + y)) * 2 + layer_ptr_int) = (ushort)color_idx;
      if (center_x - y < width && center_y + x < height)
          *(ushort *)(((uint)width * (uint)(center_y + x) + (uint)(center_x - y)) * 2 + layer_ptr_int) = (ushort)color_idx;
      if (center_x + y < width && center_y - x < height)
          *(ushort *)(((uint)width * (uint)(center_y - x) + (uint)(center_x + y)) * 2 + layer_ptr_int) = (ushort)color_idx;
      if (center_x - y < width && center_y - x < height)
          *(ushort *)(((uint)width * (uint)(center_y - x) + (uint)(center_x - y)) * 2 + layer_ptr_int) = (ushort)color_idx;
    }
  } else { // Filled circle (using vertical lines)
    int x = 0;
    int y = radius;
    int d = 1 - radius;
    int delta_e = 3;
    int delta_se = -2 * (int)radius + 5;

    vgf_vline_helper(center_x, center_y - radius, radius * 2, layer_ptr_int, color_idx, width, height);
    
    while (x < y) {
      if (d < 0) {
        d += delta_e;
        delta_e += 2;
        delta_se += 2;
      } else {
        d += delta_se;
        delta_e += 2;
        delta_se += 4;
        y--;
      }
      x++;

      vgf_vline_helper(center_x + x, center_y - y, y * 2, layer_ptr_int, color_idx, width, height);
      vgf_vline_helper(center_x + y, center_y - x, x * 2, layer_ptr_int, color_idx, width, height);
      vgf_vline_helper(center_x - x, center_y - y, y * 2, layer_ptr_int, color_idx, width, height);
      vgf_vline_helper(center_x - y, center_y - x, x * 2, layer_ptr_int, color_idx, width, height);
    }
  }
}

// Function: vgf_render_triangle
void vgf_render_triangle(ushort *coords, int layer_ptr_int, uint fill_flag, byte color_idx, ushort width, ushort height) {
  if (coords == NULL) return;

  ushort p0x_orig = coords[0];
  ushort p0y_orig = coords[1];
  ushort p1x_orig = coords[2];
  ushort p1y_orig = coords[3];
  ushort p2x_orig = coords[4];
  ushort p2y_orig = coords[5];

  // Basic bounds check (simplified for initial points, actual drawing will clip)
  if (p0x_orig >= width || p0y_orig >= height || p1x_orig >= width || p1y_orig >= height || p2x_orig >= width || p2y_orig >= height) {
    return;
  }

  if ((fill_flag & 1) == 0) { // Wireframe
    vgf_line_helper(p0x_orig, p0y_orig, p1x_orig, p1y_orig, layer_ptr_int, color_idx, width, height);
    vgf_line_helper(p1x_orig, p1y_orig, p2x_orig, p2y_orig, layer_ptr_int, color_idx, width, height);
    vgf_line_helper(p2x_orig, p2y_orig, p0x_orig, p0y_orig, layer_ptr_int, color_idx, width, height);
  } else { // Filled triangle (original scanline fill logic)
    // Sort vertices by y-coordinate (p0y <= p1y <= p2y)
    ushort p0x = p0x_orig, p0y = p0y_orig;
    ushort p1x = p1x_orig, p1y = p1y_orig;
    ushort p2x = p2x_orig, p2y = p2y_orig;
    ushort tx, ty;

    // Sort P0, P1, P2 by y-coordinate
    if (p0y > p1y) { tx=p0x; p0x=p1x; p1x=tx; ty=p0y; p0y=p1y; p1y=ty; }
    if (p0y > p2y) { tx=p0x; p0x=p2x; p2x=tx; ty=p0y; p0y=p2y; p2y=ty; }
    if (p1y > p2y) { tx=p1x; p1x=p2x; p2x=tx; ty=p1y; p1y=p2y; p2y=ty; }

    // If all points have same y-coordinate, it's a horizontal line
    if (p0y == p2y) {
        ushort min_x = p0x;
        ushort max_x = p0x;
        if (p1x < min_x) min_x = p1x; else if (p1x > max_x) max_x = p1x;
        if (p2x < min_x) min_x = p2x; else if (p2x > max_x) max_x = p2x;
        vgf_hline_helper(min_x, p0y, max_x - min_x, layer_ptr_int, color_idx, width, height);
    } else {
        // Calculate deltas for edges
        int dx02 = p2x - p0x;
        int dy02 = p2y - p0y;
        int dx01 = p1x - p0x;
        int dy01 = p1y - p0y;
        int dx12 = p2x - p1x;
        int dy12 = p2y - p1y;

        int current_dx_accum_01 = 0; // Accumulator for P0P1 edge
        int current_dx_accum_02 = 0; // Accumulator for P0P2 edge

        // Top half of triangle (from p0y to p1y-1)
        ushort y_loop_end_top = (p1y == p2y) ? p1y : p1y - 1;

        // If P0.y == P1.y, it's a top-flat triangle. Draw the horizontal line at p0y.
        if (p0y == p1y) {
            ushort min_x = p0x;
            ushort max_x = p0x;
            if (p1x < min_x) min_x = p1x; else if (p1x > max_x) max_x = p1x;
            vgf_hline_helper(min_x, p0y, max_x - min_x, layer_ptr_int, color_idx, width, height);
        }
        
        for (ushort scanline_y = p0y; scanline_y <= y_loop_end_top; ++scanline_y) {
            // Skip if p0y == p1y and this would draw the same line again
            if (p0y == p1y && scanline_y == p0y) continue; 

            ushort x_intersect_02, x_intersect_01;

            if (dy02 != 0) {
                x_intersect_02 = p0x + (current_dx_accum_02 / dy02);
            } else {
                x_intersect_02 = p0x; // Should not happen if p0y != p2y
            }
            if (dy01 != 0) {
                x_intersect_01 = p0x + (current_dx_accum_01 / dy01);
            } else {
                x_intersect_01 = p0x; // Should not happen if p0y != p1y
            }
            
            current_dx_accum_02 += dx02;
            current_dx_accum_01 += dx01;

            ushort hline_start_x = x_intersect_01;
            ushort hline_end_x = x_intersect_02;
            if (x_intersect_02 < x_intersect_01) {
                hline_start_x = x_intersect_02;
                hline_end_x = x_intersect_01;
            }
            vgf_hline_helper(hline_start_x, scanline_y, hline_end_x - hline_start_x, layer_ptr_int, color_idx, width, height);
        }

        // Re-initialize accumulators for bottom part (from p1y to p2y)
        // current_dx_accum_02 should be dx02 * (p1y - p0y) for the P0P2 edge at y = p1y
        current_dx_accum_02 = dx02 * (p1y - p0y); 
        // current_dx_accum_01 should be dx12 * (p1y - p1y) for the P1P2 edge at y = p1y, which is 0
        current_dx_accum_01 = 0; 
        
        // Bottom half of triangle (from p1y to p2y)
        for (ushort scanline_y = p1y; scanline_y <= p2y; ++scanline_y) {
            ushort x_intersect_02, x_intersect_12;

            if (dy02 != 0) {
                x_intersect_02 = p0x + (current_dx_accum_02 / dy02); // Edge P0P2
            } else {
                x_intersect_02 = p0x;
            }
            if (dy12 != 0) {
                x_intersect_12 = p1x + (current_dx_accum_01 / dy12); // Edge P1P2
            } else {
                x_intersect_12 = p1x;
            }
            
            current_dx_accum_02 += dx02;
            current_dx_accum_01 += dx12;

            ushort hline_start_x = x_intersect_02;
            ushort hline_end_x = x_intersect_12;
            if (x_intersect_12 < x_intersect_02) {
                hline_start_x = x_intersect_12;
                hline_end_x = x_intersect_02;
            }
            vgf_hline_helper(hline_start_x, scanline_y, hline_end_x - hline_start_x, layer_ptr_int, color_idx, width, height);
        }
    }
  }
}

// Function: vgf_render_line
void vgf_render_line(ushort *coords, int layer_ptr_int, uint fill_flag, byte color_idx, ushort width, ushort height) {
  if (coords == NULL) return;

  ushort x0 = coords[0];
  ushort y0 = coords[1];
  ushort x1 = coords[2];
  ushort y1 = coords[3];
  
  // Basic bounds check
  if (x0 >= width || y0 >= height || x1 >= width || y1 >= height) {
    return;
  }
  
  // The original fill_flag for line doesn't affect drawing, just calls line_helper
  vgf_line_helper(x0, y0, x1, y1, layer_ptr_int, color_idx, width, height);
}