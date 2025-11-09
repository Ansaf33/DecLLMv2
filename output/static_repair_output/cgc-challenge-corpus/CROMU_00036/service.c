#include <stdint.h> // For uint32_t, uint16_t, uint8_t
#include <stdio.h>  // For fprintf, printf, perror
#include <stdlib.h> // For exit, malloc, free
#include <string.h> // For memset

// Custom type definitions
typedef uint32_t undefined4;
typedef uint16_t ushort;
typedef uint8_t byte;

// --- Function Prototypes ---

// ReceiveAll: A generic function to receive data.
// If out_size is non-NULL, it populates the buffer size.
// If out_buffer_ptr is non-NULL, it populates the buffer with data.
// Returns 0 on success, -1 on failure.
int ReceiveAll(ushort *out_size, byte **out_buffer_ptr);

// allocate: Allocates a memory buffer of the specified size.
// Returns 0 on success, -1 on failure.
int allocate(byte **out_buffer_ptr, ushort size);

// _terminate: Exits the program due to a fatal error.
void _terminate(void);

// VGFVerify: Verifies a VGF (Vector Graphics Format) header.
// buffer_start: The initial start of the VGF data.
// buffer_current: The current position in the buffer, expected to be after the header.
// buffer_end: The end boundary of the buffer.
// Returns 0 on success, -1 on failure.
int VGFVerify(const byte *buffer_start, const byte *buffer_current, const byte *buffer_end);

// CreateCanvas: Creates a canvas for drawing, allocating its pixel buffer.
// out_canvas_data_ptr: Pointer to store the allocated canvas pixel buffer.
// width, height, depth: Dimensions and color depth of the canvas.
// Returns 0 on success, -1 on failure.
int CreateCanvas(byte **out_canvas_data_ptr, ushort width, ushort height, byte depth);

// VGFProcess: Processes VGF data.
// canvas_data_ptr: The pixel buffer of the canvas.
// io_buffer_ptr: Pointer to the current position in the input data buffer.
//                This pointer may be advanced by the function.
// buffer_end: The end boundary of the input data buffer.
// Returns 0 on success, -1 on failure.
int VGFProcess(byte *canvas_data_ptr, byte **io_buffer_ptr, const byte *buffer_end);

// FlattenCanvas: Flattens the canvas (e.g., prepares it for output).
// canvas_data_ptr: The pixel buffer of the canvas.
void FlattenCanvas(byte *canvas_data_ptr);

// PMPGenerate: Generates PMP (Portable Map Picture) data from the canvas.
// canvas_data_ptr: The pixel buffer of the canvas.
// out_pmp_data_buffer: Buffer to store the generated PMP data.
// Returns 0 on success, -1 on failure.
int PMPGenerate(byte *canvas_data_ptr, byte *out_pmp_data_buffer);

// PMPTransmit: Transmits the generated PMP data.
// pmp_data_buffer: The buffer containing PMP data.
// Returns 0 on success, -1 on failure.
int PMPTransmit(const byte *pmp_data_buffer);

// PMPDeallocate: Deallocates resources associated with PMP data.
// pmp_data_buffer: The buffer containing PMP data.
void PMPDeallocate(byte *pmp_data_buffer);

// DestroyCanvas: Destroys the canvas and frees its allocated pixel buffer.
// canvas_data_ptr: The pixel buffer of the canvas.
void DestroyCanvas(byte *canvas_data_ptr);


// --- Dummy Implementations ---

int ReceiveAll(ushort *out_size, byte **out_buffer_ptr) {
    if (out_size != NULL) {
        *out_size = 0x1000; // Dummy size, e.g., 4KB
        printf("ReceiveAll: Populated size: %u\n", *out_size);
    }
    if (out_buffer_ptr != NULL && *out_buffer_ptr != NULL) {
        // Assume data is received into the buffer pointed to by *out_buffer_ptr
        // We'll fill it with dummy header and color data for the main function's logic.
        // Assuming the buffer is at least 0x1000 bytes as set by `*out_size`.
        memset(*out_buffer_ptr, 0xAA, 0x1000); // Fill with some pattern

        // VGF header dummy values:
        // width at offset 6, height at offset 8, depth at offset 10
        ((ushort*)((*out_buffer_ptr) + 6))[0] = 640; // Dummy width
        ((ushort*)((*out_buffer_ptr) + 8))[0] = 480; // Dummy height
        (*out_buffer_ptr)[10] = 24;                 // Dummy depth

        // Dummy count for colors (read after 0xb bytes and VGFProcess)
        // This count byte is expected at (*out_buffer_ptr)[11] based on main's logic.
        (*out_buffer_ptr)[11] = 5; // Dummy count of 5 colors (must be < 0x15)

        // Dummy color data (3 bytes each) following the count byte
        // These are accessed as current_buffer_ptr[0], [1], [2] after advancing.
        (*out_buffer_ptr)[12] = 0xFF; (*out_buffer_ptr)[13] = 0x00; (*out_buffer_ptr)[14] = 0x00; // Red
        (*out_buffer_ptr)[15] = 0x00; (*out_buffer_ptr)[16] = 0xFF; (*out_buffer_ptr)[17] = 0x00; // Green
        (*out_buffer_ptr)[18] = 0x00; (*out_buffer_ptr)[19] = 0x00; (*out_buffer_ptr)[20] = 0xFF; // Blue
        (*out_buffer_ptr)[21] = 0xFF; (*out_buffer_ptr)[22] = 0xFF; (*out_buffer_ptr)[23] = 0x00; // Yellow
        (*out_buffer_ptr)[24] = 0xFF; (*out_buffer_ptr)[25] = 0x00; (*out_buffer_ptr)[26] = 0xFF; // Magenta
        printf("ReceiveAll: Filled buffer at %p\n", (void*)*out_buffer_ptr);
    }
    return 0; // Success
}

int allocate(byte **out_buffer_ptr, ushort size) {
    if (out_buffer_ptr == NULL || size == 0) return -1;
    *out_buffer_ptr = (byte *)malloc(size);
    if (*out_buffer_ptr == NULL) {
        perror("allocate: Failed to allocate buffer");
        return -1;
    }
    printf("allocate: Buffer allocated: %u bytes at %p\n", size, (void*)*out_buffer_ptr);
    return 0; // Success
}

void _terminate(void) {
    fprintf(stderr, "_terminate: Fatal error, program terminated.\n");
    exit(1);
}

int VGFVerify(const byte *buffer_start, const byte *buffer_current, const byte *buffer_end) {
    // Dummy verification: Check if buffer_current is within bounds for basic access.
    // The actual header fields (width, height, depth) are read from `buffer_start` in main.
    // This function might perform magic number checks or CRC, etc.
    if (buffer_current >= buffer_end) {
        fprintf(stderr, "VGFVerify: Buffer current pointer past end.\n");
        return -1;
    }
    printf("VGFVerify: Header verified.\n");
    return 0; // Success
}

int CreateCanvas(byte **out_canvas_data_ptr, ushort width, ushort height, byte depth) {
    if (out_canvas_data_ptr == NULL) return -1;
    // Allocate memory for a dummy canvas pixel buffer.
    // The original code uses an offset of 0x20 for colors, implying a header in the canvas buffer.
    size_t pixel_data_size = (size_t)width * height * (depth / 8);
    *out_canvas_data_ptr = (byte *)malloc(pixel_data_size + 0x20); // Add 0x20 for assumed internal header/padding
    if (*out_canvas_data_ptr == NULL) {
        perror("CreateCanvas: Failed to allocate canvas buffer");
        return -1;
    }
    memset(*out_canvas_data_ptr, 0x00, pixel_data_size + 0x20); // Clear canvas buffer
    printf("CreateCanvas: Canvas created %ux%u, depth %u, buffer at %p\n", width, height, depth, (void*)*out_canvas_data_ptr);
    return 0; // Success
}

int VGFProcess(byte *canvas_data_ptr, byte **io_buffer_ptr, const byte *buffer_end) {
    if (io_buffer_ptr == NULL || *io_buffer_ptr == NULL || canvas_data_ptr == NULL) return -1;
    // Dummy processing: The main logic implies *io_buffer_ptr should point to the color count byte
    // after this function returns. So, it doesn't need to advance *io_buffer_ptr here.
    printf("VGFProcess: VGF data processed for canvas %p.\n", (void*)canvas_data_ptr);
    return 0; // Success
}

void FlattenCanvas(byte *canvas_data_ptr) {
    if (canvas_data_ptr == NULL) {
        fprintf(stderr, "FlattenCanvas: NULL canvas data pointer.\n");
        return;
    }
    printf("FlattenCanvas: Canvas at %p flattened.\n", (void*)canvas_data_ptr);
}

int PMPGenerate(byte *canvas_data_ptr, byte *out_pmp_data_buffer) {
    if (canvas_data_ptr == NULL || out_pmp_data_buffer == NULL) return -1;
    // Dummy generation: fill the output buffer with a pattern.
    memset(out_pmp_data_buffer, 0xCC, 62); // Assuming PMP data size is 62 bytes
    printf("PMPGenerate: PMP data generated from canvas %p to %p.\n", (void*)canvas_data_ptr, (void*)out_pmp_data_buffer);
    return 0; // Success
}

int PMPTransmit(const byte *pmp_data_buffer) {
    if (pmp_data_buffer == NULL) return -1;
    printf("PMPTransmit: PMP data transmitted from %p.\n", (const void*)pmp_data_buffer);
    return 0; // Success
}

void PMPDeallocate(byte *pmp_data_buffer) {
    // In main, pmp_data_buffer (local_76) is a local stack array, so no dynamic memory to free.
    printf("PMPDeallocate: PMP data (local array at %p) deallocation simulated.\n", (void*)pmp_data_buffer);
}

void DestroyCanvas(byte *canvas_data_ptr) {
    if (canvas_data_ptr == NULL) {
        fprintf(stderr, "DestroyCanvas: NULL canvas data pointer.\n");
        return;
    }
    free(canvas_data_ptr);
    printf("DestroyCanvas: Canvas buffer at %p destroyed.\n", (void*)canvas_data_ptr);
}


// --- Fixed Main Function ---
undefined4 main(void) {
  ushort buffer_size = 0;
  byte *allocated_data_buffer = NULL; // Pointer to the main data buffer (from `allocate`)
  byte *current_buffer_ptr = NULL;    // Pointer that advances through `allocated_data_buffer`
  int result_status;
  uint32_t main_return_val;

  // 1. Receive initial buffer size
  result_status = ReceiveAll(&buffer_size, NULL);
  if (result_status == 0) {
    // 2. Allocate memory for the data buffer
    result_status = allocate(&allocated_data_buffer, buffer_size);
    if (result_status != 0) {
      _terminate(); // _terminate exits the program, so no return here.
    }
    current_buffer_ptr = allocated_data_buffer; // Initialize current_buffer_ptr

    const byte *buffer_end_ptr = allocated_data_buffer + buffer_size;

    // 3. Receive actual data into the allocated buffer
    result_status = ReceiveAll(NULL, &allocated_data_buffer); // Pass &allocated_data_buffer to fill it
    if (result_status == 0) {
      const byte *header_start_ptr = current_buffer_ptr; // Keep a pointer to the start of the header

      // 4. Check if enough data for initial header processing (at least 11 bytes)
      if (current_buffer_ptr + 0xb < buffer_end_ptr) {
        current_buffer_ptr = current_buffer_ptr + 0xb; // Advance pointer past initial header segment

        // 5. Verify VGF header
        result_status = VGFVerify(header_start_ptr, current_buffer_ptr, buffer_end_ptr);
        if (result_status == 0) {
          // Extract header values for canvas creation
          ushort width = *(ushort *)(header_start_ptr + 6);
          ushort height = *(ushort *)(header_start_ptr + 8);
          byte depth = header_start_ptr[10];

          byte *canvas_pixel_buffer = NULL; // Pointer to the canvas pixel data buffer
          // 6. Create canvas
          result_status = CreateCanvas(&canvas_pixel_buffer, width, height, depth);
          if (result_status == 0) {
            // 7. Process VGF data
            result_status = VGFProcess(canvas_pixel_buffer, &current_buffer_ptr, buffer_end_ptr);
            if (result_status == 0) {
              // 8. Read color count and advance pointer
              byte color_count = *current_buffer_ptr;
              current_buffer_ptr = current_buffer_ptr + 1; // Advance past color count byte

              // 9. Process color data if count is within limits (max 20 colors)
              if (color_count < 0x15) {
                int i;
                for (i = 0; i < (int)color_count; i = i + 1) {
                  // Check if enough bytes are left for a color (3 bytes: R, G, B)
                  if (current_buffer_ptr + 3 > buffer_end_ptr) {
                    main_return_val = 0xffffffff;
                    DestroyCanvas(canvas_pixel_buffer); // Cleanup canvas
                    free(allocated_data_buffer);        // Cleanup main data buffer
                    return main_return_val;             // Early exit on insufficient data
                  }
                  // Write BGR color data to canvas_pixel_buffer with 0x20 offset
                  canvas_pixel_buffer[i * 3 + 0x20 + 2] = current_buffer_ptr[2]; // Blue component
                  canvas_pixel_buffer[i * 3 + 0x20 + 1] = current_buffer_ptr[1]; // Green component
                  canvas_pixel_buffer[i * 3 + 0x20 + 0] = current_buffer_ptr[0]; // Red component
                  current_buffer_ptr = current_buffer_ptr + 3;                   // Advance past current color data
                }

                byte pmp_data_buffer[62]; // Local array for PMP data
                // 10. Flatten canvas
                FlattenCanvas(canvas_pixel_buffer);
                // 11. Generate PMP data
                result_status = PMPGenerate(canvas_pixel_buffer, pmp_data_buffer);
                if (result_status == 0) {
                  // 12. Transmit PMP data
                  result_status = PMPTransmit(pmp_data_buffer);
                  if (result_status == 0) {
                    PMPDeallocate(pmp_data_buffer);    // Dummy deallocation for local array
                    DestroyCanvas(canvas_pixel_buffer); // Frees canvas_pixel_buffer
                    free(allocated_data_buffer);        // Frees allocated_data_buffer
                    main_return_val = 0;                // Success
                  }
                  else { // PMPTransmit failed
                    PMPDeallocate(pmp_data_buffer);
                    DestroyCanvas(canvas_pixel_buffer);
                    free(allocated_data_buffer);
                    main_return_val = 0xffffffff;
                  }
                }
                else { // PMPGenerate failed
                  DestroyCanvas(canvas_pixel_buffer);
                  free(allocated_data_buffer);
                  main_return_val = 0xffffffff;
                }
              }
              else { // Color count too high
                DestroyCanvas(canvas_pixel_buffer);
                free(allocated_data_buffer);
                main_return_val = 0xffffffff;
              }
            }
            else { // VGFProcess failed
              DestroyCanvas(canvas_pixel_buffer);
              free(allocated_data_buffer);
              main_return_val = 0xffffffff;
            }
          }
          else { // CreateCanvas failed
            free(allocated_data_buffer); // Canvas buffer was not allocated, so only free main data buffer
            main_return_val = 0xffffffff;
          }
        }
        else { // VGFVerify failed
          free(allocated_data_buffer);
          main_return_val = 0xffffffff;
        }
      }
      else { // Not enough data for VGF header
        free(allocated_data_buffer);
        main_return_val = 0xffffffff;
      }
    }
    else { // Second ReceiveAll failed
      free(allocated_data_buffer);
      main_return_val = 0xffffffff;
    }
  }
  else { // First ReceiveAll failed
    // No memory allocated yet, so no free needed
    main_return_val = 0xffffffff;
  }
  return main_return_val;
}