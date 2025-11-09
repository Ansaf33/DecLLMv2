#include <stdlib.h> // For malloc, free
#include <stdio.h>  // For potential setup_io or debugging (not strictly required by snippet logic)
#include <stdint.h> // For intptr_t if casting pointers to/from int, or for fixed-width types

// External function declarations (implementations not provided in snippet)
extern void setup_io(void);
extern unsigned short read_u16(void);
extern unsigned char read_u8(void);
extern void _terminate(void); // Assumed to exit the program

// Assumed signature: (data_buffer, data_length, output_status_ptr)
extern int vgf_parse_data(void* data, int len, int* out_status);

extern int vgf_get_render_size(void);

// Assumed signature: (render_buffer, render_size)
extern int vgf_render_file(void* render_buffer, int render_size);

extern unsigned int vgf_get_height(void);
extern unsigned int vgf_get_width(void);

// Assumed signature: (render_buffer, width, height, out_pixels_count, out_pixel_data_ptr)
// `out_pixel_data_ptr` is `void**` because `local_4c` (pixel_data_ptr) is later freed.
extern int pixelmap_write_file(void* render_buffer, unsigned int width, unsigned int height, unsigned int* out_pixels_count, void** out_pixel_data_ptr);

extern void write_u8(unsigned char byte_val);

extern void vgf_destroy_file(void);

// Function: main
int main(void) {
  unsigned short data_len;
  void *data_buffer = NULL;
  unsigned int i; // Loop counter
  int parse_status_val; // Value from vgf_parse_data, not directly used later but passed by reference
  int render_size;
  void *render_buffer = NULL;
  unsigned int img_height;
  unsigned int img_width;
  unsigned int pixels_count;
  void *pixel_data_ptr = NULL; // Changed from `int` to `void*` based on usage (array access, free)
  unsigned int temp_pixels_count;

  setup_io();

  data_len = read_u16();
  if (data_len == 0 || data_len > 20000) {
    _terminate();
    return 1; // Indicate error
  }

  data_buffer = malloc(data_len);
  if (data_buffer == NULL) {
    _terminate();
    return 1;
  }

  for (i = 0; i < data_len; i++) {
    ((unsigned char*)data_buffer)[i] = read_u8();
  }

  if (vgf_parse_data(data_buffer, data_len, &parse_status_val) != 0) {
    free(data_buffer);
    _terminate();
    return 1;
  }

  free(data_buffer);
  data_buffer = NULL; // Good practice after freeing

  render_size = vgf_get_render_size();
  if (render_size < 1) {
    _terminate();
    return 1;
  }

  render_buffer = malloc(render_size);
  if (render_buffer == NULL) {
    _terminate();
    return 1;
  }

  if (vgf_render_file(render_buffer, render_size) != 0) {
    free(render_buffer);
    _terminate();
    return 1;
  }

  img_height = vgf_get_height();
  img_width = vgf_get_width();

  // Arguments `img_width & 0xffff` and `img_height & 0xffff`
  // suggest that only the lower 16 bits of width/height are relevant.
  if (pixelmap_write_file(render_buffer, img_width & 0xffff, img_height & 0xffff, &pixels_count, &pixel_data_ptr) < 1) {
    free(render_buffer);
    _terminate();
    return 1;
  }

  free(render_buffer);
  render_buffer = NULL;

  temp_pixels_count = pixels_count;
  for (i = 0; i < temp_pixels_count; i++) {
    write_u8(((unsigned char*)pixel_data_ptr)[i]);
  }

  free(pixel_data_ptr);
  pixel_data_ptr = NULL;

  vgf_destroy_file();

  return 0; // Success
}