#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h> // For strtod (though it's in stdlib.h, sometimes math.h is included for related functions)

// Define custom types based on common reverse engineering tools output
typedef uint8_t byte;
typedef uint8_t undefined;
typedef uint8_t undefined1;
typedef uint32_t undefined4; // Assuming 4-byte unsigned integer
typedef double undefined8; // Assuming 8-byte double for floating point operations

// Global buffer used by fread_until and strtok
// Assuming its size based on `fread_until(&buf,10,0x1000,stdin);`
char buf[0x1000];

// --- Type Definitions for Objects ---

// Generic Vector structure
typedef struct {
    double x, y, z;
} Vector;

// Generic Object for linked list in main
typedef struct GenericObject {
    struct GenericObject *next; // Pointer to the next object in a linked list
    int type;                   // 0 for sphere, 1 for plane, -1 for uninitialized
    // Other object-specific data will follow this header
    // Max size needed for plane/sphere data is 0x6C for PlaneObject.
    // Allocate enough space for the largest object type.
    uint8_t data[0x6C - sizeof(struct GenericObject*) - sizeof(int)];
} GenericObject;

// Sphere specific structure, overlayed on GenericObject data
typedef struct {
    // Note: The first 8 bytes (offset 0-7) are likely padding/unused in the object structure,
    // or contain other generic object data not explicitly shown (e.g., vtable pointer).
    uint8_t _padding_0_8[8]; // Padding or unknown data
    uint32_t type_flag;      // offset 8, 0 for diffuse, 1 for specular
    Vector position;         // offset 0xc (12)
    Vector color;            // offset 0x24 (36)
    Vector emission;         // offset 0x3c (60)
    uint8_t _padding_54_57[4]; // Padding from 0x54 to 0x57
    float radius;            // offset 0x58 (88)
} SphereObject; // Total size 0x58 + sizeof(float) = 0x5C

// Plane specific structure, overlayed on GenericObject data
typedef struct {
    uint8_t _padding_0_8[8]; // Padding or unknown data
    uint32_t type_flag;      // offset 8, 0 for diffuse, 1 for specular
    Vector position;         // offset 0xc (12)
    Vector color;            // offset 0x24 (36)
    Vector emission;         // offset 0x3c (60)
    Vector normal;           // offset 0x54 (84)
} PlaneObject; // Total size 0x54 + sizeof(Vector) = 0x54 + 24 = 0x6C

// Path Tracer Context structure (stack-allocated in main)
typedef struct {
    char data[64]; // Size based on `undefined local_90 [64];`
} PtContext;

// --- Stub Functions ---
// These functions are not provided in the snippet, but are called.
// They are declared here with minimal placeholder implementations to allow compilation.

byte get_flag_byte(uint32_t index) {
    // Placeholder implementation, return a dummy byte
    return (byte)(index & 0xFF);
}

int fread_until(char *buffer, int delimiter, size_t max_size, FILE *stream) {
    // Placeholder: read a line from stdin
    if (fgets(buffer, max_size, stream) != NULL) {
        // Remove newline character if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        // Simplified return: length of string read. Original might have more complex logic.
        return (int)strlen(buffer);
    }
    return -1; // Error or EOF
}

void make_vector(Vector *out_vec, double x, double y, double z) {
    out_vec->x = x;
    out_vec->y = y;
    out_vec->z = z;
}

void sphere_init(void *sphere_obj_ptr, double radius) {
    printf("sphere_init called with ptr %p, radius %f\n", sphere_obj_ptr, radius);
}

void plane_init(void *plane_obj_ptr, undefined4 n_x1, undefined4 n_x2, undefined4 n_y1, undefined4 n_y2, undefined4 n_z1, undefined4 n_z2) {
    printf("plane_init called with ptr %p and normal vector components...\n", plane_obj_ptr);
}

int allocate(size_t size, int flags, void **out_ptr) {
    *out_ptr = malloc(size);
    if (*out_ptr == NULL) return -1;
    printf("allocate called, size %zu, ptr %p\n", size, *out_ptr);
    return 0;
}

int image_init(void *image_ptr, int width, int height) {
    int *img = (int*)image_ptr;
    if (img) {
        img[0] = width;
        img[1] = height;
        printf("image_init called, ptr %p, width %d, height %d\n", image_ptr, width, height);
        // Initialize dummy pixel data (starts at offset 8 after width/height)
        uint8_t *pixels = (uint8_t*)image_ptr + 8;
        for (int i = 0; i < width * height * 3; ++i) {
            pixels[i] = (uint8_t)(i % 255);
        }
    }
    return 0;
}

int pt_init(void *pt_context_ptr) {
    printf("pt_init called, ptr %p\n", pt_context_ptr);
    return 0;
}

void *pool_alloc() {
    GenericObject *obj = (GenericObject*)malloc(sizeof(GenericObject));
    if (obj) {
        obj->next = NULL;
        obj->type = -1; // Default uninitialized type
        printf("pool_alloc called, ptr %p\n", obj);
    }
    return obj;
}

void pt_render(void *image_ptr, void *pt_context_ptr) {
    printf("pt_render called, image_ptr %p, pt_context_ptr %p\n", image_ptr, pt_context_ptr);
}

void pt_clear_ctx(void *pt_context_ptr) {
    printf("pt_clear_ctx called, ptr %p\n", pt_context_ptr);
}

// Ray structure for camera setup
typedef struct {
    Vector origin;
    Vector direction;
} Ray;

void make_ray(Ray *out_ray, Vector *pos, Vector *dir, double fov) {
    out_ray->origin = *pos;
    out_ray->direction = *dir;
    printf("make_ray called with origin (%f,%f,%f), direction (%f,%f,%f), fov %f\n",
           pos->x, pos->y, pos->z, dir->x, dir->y, dir->z, fov);
}

void pt_setup_camera(void *pt_context_ptr, Ray *camera_ray, double fov) {
    printf("pt_setup_camera called, ctx %p, ray origin (%f,%f,%f), fov %f\n",
           pt_context_ptr, camera_ray->origin.x, camera_ray->origin.y, camera_ray->origin.z, fov);
}

void pt_destroy(void *pt_context_ptr) {
    printf("pt_destroy called, ptr %p\n", pt_context_ptr);
    // As pt_context_ptr is stack allocated in main, do NOT free it here.
    // Assume it cleans up internal resources associated with the context.
}

void image_destroy(void *image_ptr) {
    printf("image_destroy called, ptr %p\n", image_ptr);
}

void deallocate(void *ptr, size_t size) {
    printf("deallocate called, ptr %p, size %zu\n", ptr, size);
    free(ptr);
}

// --- End Stub Functions ---

// Function: calculate_csum
byte calculate_csum(int *param_1) {
  byte checksum = 0;
  char *pixel_data_base = (char *)param_1 + 8; // Pixel data starts after width/height (2 ints = 8 bytes)

  for (uint32_t i = 0; i < 0x400; ++i) {
    checksum ^= get_flag_byte(i);
  }

  uint32_t total_pixels = (uint32_t)param_1[1] * param_1[0]; // height * width
  for (uint32_t i = 0; i < total_pixels; ++i) {
    // Accessing R, G, B components of the i-th pixel
    checksum ^= pixel_data_base[i * 3 + 2] ^ // Blue component (offset +2 from pixel start)
                pixel_data_base[i * 3 + 0] ^ // Red component (offset +0 from pixel start)
                pixel_data_base[i * 3 + 1];  // Green component (offset +1 from pixel start)
  }
  return checksum;
}

// Function: write_ppm
void write_ppm(int *param_1) {
  uint32_t width = (uint32_t)param_1[0];
  uint32_t height = (uint32_t)param_1[1];
  printf("%d %d\n", width, height);

  char *pixel_data_base = (char *)param_1 + 8;

  uint32_t total_pixels = width * height;
  for (uint32_t i = 0; i < total_pixels; ++i) {
    uint32_t r = pixel_data_base[i * 3 + 0];
    uint32_t g = pixel_data_base[i * 3 + 1];
    uint32_t b = pixel_data_base[i * 3 + 2];
    printf("%u %u %u ", r, g, b); // Use %u for unsigned byte values
  }
  uint32_t csum = calculate_csum(param_1);
  printf("\n%x\n", csum); // Removed unused arguments
}

// Function: read_vector
undefined4 read_vector(Vector *param_1) {
  int iVar1;
  char *token;
  char *endptr;

  iVar1 = fread_until(buf, ',', sizeof(buf), stdin); // Reads until comma or newline
  if (iVar1 == -1) {
    return 0xffffffff;
  }

  double x, y, z;

  token = strtok(buf, ", ");
  if (token == NULL || strlen(token) == 0) return 0xffffffff;
  x = strtod(token, &endptr);
  if (endptr == token) return 0xffffffff;

  token = strtok(NULL, ", ");
  if (token == NULL || strlen(token) == 0) return 0xffffffff;
  y = strtod(token, &endptr);
  if (endptr == token) return 0xffffffff;

  token = strtok(NULL, ", ");
  if (token == NULL || strlen(token) == 0) return 0xffffffff;
  z = strtod(token, &endptr);
  if (endptr == token) return 0xffffffff;

  make_vector(param_1, x, y, z);
  return 0;
}

// Function: read_double
undefined4 read_double(double *param_1) {
  int iVar1;
  char *endptr;

  iVar1 = fread_until(buf, 10, sizeof(buf), stdin); // Reads until newline
  if (iVar1 == -1) {
    return 0xffffffff;
  }

  *param_1 = strtod(buf, &endptr);
  if (endptr == buf || *endptr != '\0') { // Check for conversion failure or leftover characters
    return 0xffffffff;
  }
  return 0;
}

// Function: read_sphere
undefined4 read_sphere(SphereObject *sphere_obj) {
  int iVar1;

  printf("Enter \'d\' for diffuse, \'s\' for specular:\n");
  iVar1 = fread_until(buf, 10, sizeof(buf), stdin);
  if (iVar1 == -1) {
    return 0xffffffff;
  }

  if (buf[0] == 'd' || buf[0] == 's') {
    sphere_obj->type_flag = (uint32_t)(buf[0] != 'd'); // 0 for diffuse, 1 for specular
    printf("Enter radius:\n");
    iVar1 = read_double((double*)&sphere_obj->radius); // Cast float* to double* for read_double
    if (iVar1 == -1) return 0xffffffff;

    printf("Enter position as \"x, y, z\":\n");
    iVar1 = read_vector(&sphere_obj->position);
    if (iVar1 == -1) return 0xffffffff;

    printf("Enter color as \"r, g, b\":\n");
    iVar1 = read_vector(&sphere_obj->color);
    if (iVar1 == -1) return 0xffffffff;

    printf("Enter emission as \"r, g, b\":\n");
    iVar1 = read_vector(&sphere_obj->emission);
    if (iVar1 == -1) return 0xffffffff;

    sphere_init(sphere_obj, (double)sphere_obj->radius);
    return 0;
  } else {
    return 0xffffffff;
  }
}

// Function: read_plane
undefined4 read_plane(PlaneObject *plane_obj) {
  int iVar1;

  printf("Enter \'d\' for diffuse, \'s\' for specular:\n");
  iVar1 = fread_until(buf, 10, sizeof(buf), stdin);
  if (iVar1 == -1) {
    return 0xffffffff;
  }

  if (buf[0] == 'd' || buf[0] == 's') {
    plane_obj->type_flag = (uint32_t)(buf[0] != 'd');
    printf("Enter normal as \"x, y, z\":\n");
    iVar1 = read_vector(&plane_obj->normal);
    if (iVar1 == -1) return 0xffffffff;

    printf("Enter position as \"x, y, z\":\n");
    iVar1 = read_vector(&plane_obj->position);
    if (iVar1 == -1) return 0xffffffff;

    printf("Enter color as \"r, g, b\":\n");
    iVar1 = read_vector(&plane_obj->color);
    if (iVar1 == -1) return 0xffffffff;

    printf("Enter emission as \"r, g, b\":\n");
    iVar1 = read_vector(&plane_obj->emission);
    if (iVar1 == -1) return 0xffffffff;

    // Pass the 6 undefined4 components of the normal vector
    plane_init(plane_obj,
               ((undefined4*)&plane_obj->normal)[0],
               ((undefined4*)&plane_obj->normal)[1],
               ((undefined4*)&plane_obj->normal)[2],
               ((undefined4*)&plane_obj->normal)[3],
               ((undefined4*)&plane_obj->normal)[4],
               ((undefined4*)&plane_obj->normal)[5]);
    return 0;
  } else {
    return 0xffffffff;
  }
}

// Function: main
undefined4 main(void) {
  int result;
  void *image_buffer = NULL;
  PtContext pt_ctx_stack; // Stack-allocated context for path tracer
  GenericObject *current_object_being_read = NULL;
  GenericObject *object_list_head = NULL; // Head of the linked list of objects

  Vector camera_pos;
  Vector camera_dir;
  double camera_fov;
  Ray camera_ray;

  size_t image_buffer_size = 0x12c8; // 4808 bytes (40x40 image + 8 bytes header)

  result = allocate(image_buffer_size, 0, &image_buffer);
  if (result == -1) {
    return 0xffffffff;
  }

  result = image_init(image_buffer, 0x28, 0x28); // 40x40 image
  if (result == -1) {
    deallocate(image_buffer, image_buffer_size);
    return 0xffffffff;
  }

  result = pt_init(&pt_ctx_stack);
  if (result == -1) {
    image_destroy(image_buffer);
    deallocate(image_buffer, image_buffer_size);
    return 0xffffffff;
  }

  char choice;
  bool running = true;
  while (running) {
    printf("Enter \'s\' for sphere, \'p\' for plane, \'c\' for camera, \'r\' for render, \'q\' for quit\n");
    result = fread_until(buf, 10, sizeof(buf), stdin); // Read until newline
    if (result == -1 || result < 1) {
        printf("Error reading input or empty input, please try again.\n");
        continue; // Re-prompt
    }

    choice = buf[0]; // Take the first character of the input

    switch (choice) {
      case 'q':
        running = false;
        break;
      case 's':
        current_object_being_read = pool_alloc();
        if (current_object_being_read == NULL) {
            printf("Failed to allocate sphere object.\n");
            break;
        }
        current_object_being_read->type = 0; // Type 0 for sphere
        result = read_sphere((SphereObject*)current_object_being_read);
        if (result == -1) {
          printf("Invalid sphere parameters.\n");
          free(current_object_being_read); // Free on error
        } else {
          current_object_being_read->next = object_list_head; // Add to linked list
          object_list_head = current_object_being_read;
        }
        current_object_being_read = NULL; // Reset for next allocation
        break;
      case 'p':
        current_object_being_read = pool_alloc();
        if (current_object_being_read == NULL) {
            printf("Failed to allocate plane object.\n");
            break;
        }
        current_object_being_read->type = 1; // Type 1 for plane
        result = read_plane((PlaneObject*)current_object_being_read);
        if (result == -1) {
          printf("Invalid plane parameters.\n");
          free(current_object_being_read); // Free on error
        } else {
          current_object_being_read->next = object_list_head; // Add to linked list
          object_list_head = current_object_being_read;
        }
        current_object_being_read = NULL; // Reset for next allocation
        break;
      case 'c':
        printf("Enter camera position as \"x, y, z\":\n");
        if (read_vector(&camera_pos) == -1) {
          printf("Invalid camera position.\n");
          break;
        }
        printf("Enter camera direction as \"x, y, z\":\n");
        if (read_vector(&camera_dir) == -1) {
          printf("Invalid camera direction.\n");
          break;
        }
        printf("Enter camera fov:\n");
        if (read_double(&camera_fov) == -1) {
          printf("Invalid camera fov.\n");
          break;
        }
        make_ray(&camera_ray, &camera_pos, &camera_dir, camera_fov);
        pt_setup_camera(&pt_ctx_stack, &camera_ray, camera_fov);
        break;
      case 'r':
        pt_render(image_buffer, &pt_ctx_stack);
        write_ppm(image_buffer);
        pt_clear_ctx(&pt_ctx_stack);
        break;
      default:
        printf("No such command!\n");
        break;
    }
  }

  // Cleanup
  pt_destroy(&pt_ctx_stack); // Cleans up internal resources, but doesn't free the stack variable
  image_destroy(image_buffer);
  deallocate(image_buffer, image_buffer_size);

  // Free objects in the linked list
  GenericObject *current = object_list_head;
  while (current != NULL) {
      GenericObject *next = current->next;
      free(current);
      current = next;
  }

  return 0;
}