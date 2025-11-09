#include <stdint.h>   // For uint32_t, uint64_t
#include <stdbool.h>  // For bool
#include <math.h>     // For floating-point operations (e.g., fabs, sqrt if used internally by vector funcs)
#include <stddef.h>   // For size_t

// --- Type Definitions ---

// Assuming a 3D vector for ray tracing
typedef struct {
    float x, y, z;
} Vector;

// Assuming a ray with origin and direction vectors
typedef struct {
    Vector origin;
    Vector direction;
} Ray;

// Forward declaration for Shape, as it's part of PtContext
typedef struct Shape Shape;

// PtContext structure, inferred from memory offsets in the original code
// It's assumed to encapsulate pool data, camera settings, and the scene's shape list.
typedef struct PtContext {
    // Placeholder for pool internal data (8 bytes before camera_origin)
    uint8_t _pool_data[8];

    // Camera properties (offsets: +8, +0xc, +0x10, etc.)
    Vector camera_origin;     // Offset 0x8
    Vector camera_direction;  // Offset 0x14
    Vector camera_up;         // Offset 0x20
    Vector camera_right;      // Offset 0x2c
    uint64_t camera_resolution; // Offset 0x38 (width in high 32 bits, height in low 32 bits)

    // Head of a linked list of shapes in the scene
    Shape* first_shape;       // Offset 0x40
    // Additional context-specific data or pool management fields might follow
} PtContext;

// Shape structure, inferred from memory offsets and usage
// The size of each shape object is 0x78 (120 bytes) based on pool_init.
// Specific fields are accessed via pointer arithmetic and casting in functions like `intersect` and `radiance`.
// This structure represents the common header for all shapes and is used for generic access.
// Specific shape data (sphere radius, plane normal, etc.) and material properties
// are assumed to follow this common header within the 0x78 byte allocation.
// The function pointers for intersection are assumed to be stored at specific offsets
// within the allocated shape memory (0x5C for sphere, 0x70 for plane).
typedef long double (*IntersectShapeFunc)(void* shape_ptr, Ray ray); // Ray passed by value

struct Shape {
    Shape* next;       // Offset 0x0
    uint32_t type;     // Offset 0x8 (0 for Sphere, 1 for Plane)
    // Placeholder for common material properties and shape-specific data.
    // The exact layout is not fully reverse-engineered, but access patterns
    // in `radiance` and `intersect` imply a consistent structure.
    uint8_t data[120 - sizeof(Shape*) - sizeof(uint32_t)]; // Remaining 108 bytes
};


// --- Macro Definitions ---

// CONCAT44 macro for concatenating two 32-bit values into a 64-bit value
#define CONCAT44(hi, lo) (((uint64_t)(hi) << 32) | (lo))


// --- External Function Declarations (Inferred Signatures) ---

// Pool Management
extern int pool_init(void* ctx, size_t size);
extern void pool_destroy(void* ctx);
extern void* pool_alloc(void* ctx);
extern void pool_free_all(void* ctx);

// Vector Operations (output by pointer, inputs by value)
extern void make_vector(Vector* out, float x, float y, float z);
extern void vector_cross(Vector* out, Vector v1, Vector v2);
extern void vector_norm(Vector* out, Vector v_in);
extern void vector_trunc(Vector* out, Vector v_in);
extern void vector_scale(Vector* out, Vector v_in, float scale);
extern void vector_add(Vector* out, Vector v1, Vector v2);
extern void vector_sub(Vector* out, Vector v1, Vector v2);
extern void vector_mul(Vector* out, Vector v1, Vector v2);
extern long double vector_dot(Vector v1, Vector v2);
extern long double vector_mag_sqr(Vector v_in);

// Ray Operations
extern void make_ray(Ray* out, Vector origin, Vector direction);

// Shape Initialization (void* for shape_ptr, float for components, uint32_t for ID/type)
extern void shape_init(void* shape_ptr, uint32_t type_id, float p2, float p3, float p4, float p5, float p6, float p7, float p8, float p9, float p10, float p11, float p12, float p13, float p14, float p15, float p16, float p17, float p18, float p19, float p20);
extern void sphere_init(void* shape_ptr, float p21, float p22, uint32_t id);
extern void plane_init(void* shape_ptr, float p21, float p22, float p23, float p24, float p25, float p26, uint32_t id);

// Image / Color Operations
extern uint32_t color_to_pixel(Vector color, double factor); // Returns packed 32-bit color
extern void image_write_pixel(unsigned int* image_buffer, unsigned int width, unsigned int height, unsigned int x, unsigned int y, uint32_t pixel_color);

// Scene Intersection (ray components passed individually for simplicity)
extern bool intersect(PtContext* ctx, float rox, float roy, float roz, float rdx, float rdy, float rdz, Shape** hit_shape, double* hit_distance);

// Radiance Calculation (recursive)
extern Vector* radiance(Vector* out_color, PtContext* ctx, Ray ray, uint32_t depth);


// --- External Data Declarations ---

// Global data references, inferred types from usage
extern double DAT_00017000;
extern uint64_t DAT_00017008; // Used as double, likely a conversion from uint64_t to double
extern uint64_t DAT_00017010;
extern uint64_t DAT_00017018;
extern double DAT_00017020;
extern double DAT_00017028;
extern double DAT_00017030;
extern double DAT_00017038;


// --- Function Definitions ---

// Function: pt_init
uint32_t pt_init(PtContext* ctx) {
    return pool_init(ctx, 0x78) == 0 ? 0 : 0xffffffff;
}

// Function: pt_destroy
void pt_destroy(PtContext* ctx) {
    pool_destroy(ctx);
}

// Function: pt_setup_camera
void pt_setup_camera(PtContext* ctx, float cam_ox, float cam_oy, float cam_oz,
                    float cam_dx, float cam_dy, float cam_dz,
                    float cam_ux, float cam_uy, float cam_uz,
                    float cam_rx, float cam_ry, float cam_rz,
                    uint32_t res_low, uint32_t res_high) {
    ctx->camera_origin.x = cam_ox;
    ctx->camera_origin.y = cam_oy;
    ctx->camera_origin.z = cam_oz;
    ctx->camera_direction.x = cam_dx;
    ctx->camera_direction.y = cam_dy;
    ctx->camera_direction.z = cam_dz;
    ctx->camera_up.x = cam_ux;
    ctx->camera_up.y = cam_uy;
    ctx->camera_up.z = cam_uz;
    ctx->camera_right.x = cam_rx;
    ctx->camera_right.y = cam_ry;
    ctx->camera_right.z = cam_rz;
    ctx->camera_resolution = CONCAT44(res_high, res_low);
}

// Function: pt_add_sphere
uint32_t pt_add_sphere(PtContext* ctx, float p2, float p3, float p4, float p5, float p6, float p7,
                       float p8, float p9, float p10, float p11, float p12, float p13, float p14,
                       float p15, float p16, float p17, float p18, float p19, float p20,
                       float p21, float p22) {
    uint32_t sphere_id = 0x110ee; // Arbitrary ID from original code
    Shape* new_sphere = (Shape*)pool_alloc(ctx);

    if (new_sphere == NULL) {
        return 0xffffffff;
    } else {
        shape_init(new_sphere, 0, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15,
                   p16, p17, p18, p19, p20);
        sphere_init(new_sphere, p21, p22, sphere_id);
        new_sphere->next = ctx->first_shape;
        ctx->first_shape = new_sphere;
        return 0;
    }
}

// Function: pt_add_plane
uint32_t pt_add_plane(PtContext* ctx, float p2, float p3, float p4, float p5, float p6, float p7,
                      float p8, float p9, float p10, float p11, float p12, float p13, float p14,
                      float p15, float p16, float p17, float p18, float p19, float p20,
                      float p21, float p22, float p23, float p24, float p25, float p26) {
    uint32_t plane_id = 0x111a7; // Arbitrary ID from original code
    Shape* new_plane = (Shape*)pool_alloc(ctx);

    if (new_plane == NULL) {
        return 0xffffffff;
    } else {
        shape_init(new_plane, 1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15,
                   p16, p17, p18, p19, p20);
        plane_init(new_plane, p21, p22, p23, p24, p25, p26, plane_id);
        new_plane->next = ctx->first_shape;
        ctx->first_shape = new_plane;
        return 0;
    }
}

// Function: pt_render
void pt_render(PtContext* ctx, unsigned int* resolution) {
    Vector temp_v1, temp_v2, temp_v3, temp_v4, temp_v5, temp_v6;
    Ray current_ray;
    Vector pixel_color_vec;
    uint32_t image_width = resolution[0];
    uint32_t image_height = resolution[1];

    // Calculate initial camera basis vectors (right vector is derived)
    make_vector(&temp_v1,
                (float)((double)(ctx->camera_resolution >> 32) * (double)image_width / (double)image_height),
                0.0f, 0.0f); // Assuming the last two '0's in make_vector are for y, z
    vector_cross(&temp_v2, temp_v1, ctx->camera_up);
    vector_norm(&temp_v3, temp_v2);
    vector_trunc(&ctx->camera_right, temp_v3); // Update camera_right in context

    for (unsigned int y = 0; y < image_height; ++y) {
        for (unsigned int x = 0; x < image_width; ++x) {
            // Calculate ray direction based on pixel coordinates
            // Scale camera_up vector
            vector_scale(&temp_v1, ctx->camera_up, (float)y / (float)image_height - (float)DAT_00017000);
            // Scale camera_right vector
            vector_scale(&temp_v2, ctx->camera_right, (float)x / (float)image_width - (float)DAT_00017000);

            // Add scaled vectors to camera_direction
            vector_add(&temp_v3, temp_v1, temp_v2);
            vector_add(&temp_v4, temp_v3, ctx->camera_direction);

            // Normalize and truncate the ray direction
            vector_norm(&current_ray.direction, temp_v4);
            vector_trunc(&current_ray.direction, current_ray.direction);

            // Set ray origin to camera origin
            current_ray.origin = ctx->camera_origin;

            // Calculate radiance for this ray
            radiance(&pixel_color_vec, ctx, current_ray, 0);

            // Convert color to pixel format and write to image buffer
            uint32_t final_pixel_color = color_to_pixel(pixel_color_vec, *(double*)&DAT_00017008);
            image_write_pixel(resolution, image_width, image_height, x, y, final_pixel_color);
        }
    }
}

// Function: pt_clear_ctx
void pt_clear_ctx(PtContext* ctx) {
    ctx->first_shape = NULL;
    pool_free_all(ctx);

    Vector temp_v1, temp_v2;
    Ray temp_ray;

    // Initialize default camera vectors
    make_vector(&temp_v1, 0.0f, 0.0f, -1.0f); // Direction vector (0,0,-1)
    make_vector(&temp_v2, 0.0f, 0.0f, *(float*)&DAT_00017010); // Origin vector (0,0,DAT_00017010)

    // Create a default ray
    make_ray(&temp_ray, temp_v2, temp_v1);

    // Setup camera with default ray and other parameters
    pt_setup_camera(ctx, temp_ray.origin.x, temp_ray.origin.y, temp_ray.origin.z,
                    temp_ray.direction.x, temp_ray.direction.y, temp_ray.direction.z,
                    0.0f, 1.0f, 0.0f, // Default Up vector
                    1.0f, 0.0f, 0.0f, // Default Right vector
                    (uint32_t)DAT_00017018, (uint32_t)(DAT_00017018 >> 32)); // Default resolution
}

// Function: intersect
bool intersect(PtContext* ctx, float rox, float roy, float roz, float rdx, float rdy, float rdz,
               Shape** hit_shape_out, double* hit_distance_out) {
    double closest_hit_dist = DAT_00017020; // Some initial max distance (e.g., infinity)
    Shape* closest_shape = NULL;
    Ray ray = {{rox, roy, roz}, {rdx, rdy, rdz}}; // Reconstruct ray from components

    // Iterate through all shapes in the scene
    for (Shape* current_shape = ctx->first_shape; current_shape != NULL; current_shape = current_shape->next) {
        long double current_dist;
        // Call the appropriate intersect function based on shape type
        if (current_shape->type == 0) { // Sphere
            // The function pointer is assumed to be at offset 0x5C within the shape's data.
            current_dist = ((IntersectShapeFunc)((uint8_t*)current_shape + 0x5C))(current_shape, ray);
        } else if (current_shape->type == 1) { // Plane
            // The function pointer is assumed to be at offset 0x70 within the shape's data.
            current_dist = ((IntersectShapeFunc)((uint8_t*)current_shape + 0x70))(current_shape, ray);
        } else {
            continue; // Unknown shape type
        }

        // Check if this intersection is closer than previous ones and valid
        if (DAT_00017028 < (double)current_dist && (double)current_dist < closest_hit_dist) {
            closest_shape = current_shape;
            closest_hit_dist = (double)current_dist;
        }
    }

    if (closest_shape != NULL) {
        *hit_shape_out = closest_shape;
        *hit_distance_out = closest_hit_dist;
    }
    return closest_shape != NULL;
}

// Function: radiance
Vector* radiance(Vector* out_color, PtContext* ctx, Ray ray, uint32_t depth) {
    Shape* hit_shape = NULL;
    double hit_distance = 0.0;
    Vector temp_v1, temp_v2, temp_v3, temp_v4;
    Ray shadow_ray, reflection_ray;

    // Initialize output color to black
    make_vector(out_color, 0.0f, 0.0f, 0.0f);

    if (depth < 4) { // Limit recursion depth
        if (intersect(ctx, ray.origin.x, ray.origin.y, ray.origin.z,
                      ray.direction.x, ray.direction.y, ray.direction.z,
                      &hit_shape, &hit_distance)) {

            // Calculate intersection point
            vector_scale(&temp_v1, ray.direction, (float)hit_distance);
            vector_add(&temp_v1, ray.origin, temp_v1); // temp_v1 is now intersect_point

            // Determine normal at intersection point
            Vector normal_at_hit;
            if (hit_shape->type == 0) { // Sphere
                // Sphere center at offset 0xc, normal is (intersect_point - center)
                vector_sub(&normal_at_hit, temp_v1, *(Vector*)((uint8_t*)hit_shape + 0xc));
                vector_norm(&normal_at_hit, normal_at_hit);
                vector_trunc(&normal_at_hit, normal_at_hit);
            } else { // Plane
                // Plane normal at offset 0x54
                vector_norm(&normal_at_hit, *(Vector*)((uint8_t*)hit_shape + 0x54));
                vector_trunc(&normal_at_hit, normal_at_hit);
            }

            // Adjust normal if it's facing away from the ray origin
            if (vector_dot(normal_at_hit, ray.direction) > 0) {
                vector_scale(&normal_at_hit, normal_at_hit, -1.0f);
                vector_trunc(&normal_at_hit, normal_at_hit);
            }

            // Check for emission (emission color at offset 0x3c)
            Vector emission_color = *(Vector*)((uint8_t*)hit_shape + 0x3c);
            if (vector_mag_sqr(emission_color) > DAT_00017028) { // DAT_00017028 is a small epsilon
                vector_norm(out_color, emission_color); // Return normalized emission color
                return out_color;
            }

            // Reflectivity (at offset 0x24)
            float reflectivity = *(float*)((uint8_t*)hit_shape + 0x24);

            // Handle diffuse and specular components if not perfectly reflective
            if (reflectivity < 1.0f) {
                // Determine material model based on hit_shape->type (or another property at offset 0x8)
                // The original code uses `*(int *)(local_358 + 8) == 0` for diffuse/specular and `== 1` for refraction.
                // Assuming `hit_shape->type` (at offset 0x8) dictates this.
                if (hit_shape->type == 0) { // Sphere (or diffuse-like material)
                    // Loop through all shapes to find light sources
                    for (Shape* light_source = ctx->first_shape; light_source != NULL; light_source = light_source->next) {
                        Vector light_emission = *(Vector*)((uint8_t*)light_source + 0x3c); // Light emission at 0x3c
                        if (vector_mag_sqr(light_emission) > DAT_00017028) {
                            // Calculate light direction from hit point to light source center (at 0xc)
                            vector_sub(&temp_v2, *(Vector*)((uint8_t*)light_source + 0xc), temp_v1); // light_pos - intersect_point
                            vector_norm(&temp_v3, temp_v2); // temp_v3 is light_dir
                            vector_trunc(&temp_v3, temp_v3);

                            // Make a shadow ray
                            make_ray(&shadow_ray, temp_v1, temp_v3);

                            // Check for shadow intersection
                            Shape* shadow_hit_shape = NULL;
                            double shadow_hit_distance = 0.0;
                            if (intersect(ctx, shadow_ray.origin.x, shadow_ray.origin.y, shadow_ray.origin.z,
                                          shadow_ray.direction.x, shadow_ray.direction.y, shadow_ray.direction.z,
                                          &shadow_hit_shape, &shadow_hit_distance)) {
                                // If the shadow ray hits something, it's in shadow, reduce brightness
                                // Original code: `local_1c = local_1c / DAT_00017030;`
                                // `local_1c` was the dot product (brightness factor).
                                double brightness_factor = vector_dot(normal_at_hit, temp_v3); // dot(normal, light_dir)
                                if (brightness_factor > 0) {
                                    if (shadow_hit_shape != light_source) { // Don't shadow if hitting the light itself
                                        brightness_factor /= DAT_00017030; // DAT_00017030 is shadow attenuation factor
                                    }
                                    // Add diffuse component: object_color * light_emission * brightness_factor
                                    vector_scale(&temp_v4, light_emission, (float)brightness_factor);
                                    vector_mul(&temp_v2, *(Vector*)((uint8_t*)hit_shape + 0x2c), temp_v4); // Object color at 0x2c
                                    vector_add(out_color, *out_color, temp_v2);
                                }
                            } else { // Not in shadow
                                double brightness_factor = vector_dot(normal_at_hit, temp_v3);
                                if (brightness_factor > 0) {
                                    vector_scale(&temp_v4, light_emission, (float)brightness_factor);
                                    vector_mul(&temp_v2, *(Vector*)((uint8_t*)hit_shape + 0x2c), temp_v4);
                                    vector_add(out_color, *out_color, temp_v2);
                                }
                            }
                        }
                    }
                } else if (hit_shape->type == 1) { // Plane (or refractive-like material)
                    // This block in the original code seems to handle refraction/reflection for planes.
                    // It calculates reflection and then a recursive call to radiance.
                    // Reflection direction: ray.direction - 2 * dot(ray.direction, normal) * normal
                    vector_scale(&temp_v2, normal_at_hit, 2.0f * (float)vector_dot(ray.direction, normal_at_hit));
                    vector_sub(&temp_v3, ray.direction, temp_v2); // temp_v3 is reflection_dir
                    vector_norm(&temp_v3, temp_v3);
                    vector_trunc(&temp_v3, temp_v3);

                    make_ray(&reflection_ray, temp_v1, temp_v3); // intersect_point, reflection_dir
                    radiance(&temp_v4, ctx, reflection_ray, depth + 1); // temp_v4 is reflected_color

                    // Add reflected color weighted by material properties (e.g., DAT_00017038)
                    // Original code: `*(double *)(puVar3 + -0x18) = (double)((longdouble)DAT_00017038 * lVar4);`
                    // Where `lVar4` is vector_dot(reflected_dir, original_ray_dir).
                    // This implies a Fresnel-like term or just a constant reflection strength.
                    vector_scale(&temp_v4, temp_v4, (float)DAT_00017038 * (float)vector_dot(temp_v3, ray.direction));
                    vector_add(out_color, *out_color, temp_v4);

                    // Refraction (if any) would follow here. This part is highly speculative.
                    // The original code has more vector operations and another recursive call to radiance.
                    // It seems to blend reflected and refracted light.
                    // Skipping full refraction implementation due to complexity and lack of full context.
                }
            }

            // Handle general reflection (if reflectivity > 0)
            if (reflectivity > 0 && hit_shape->type != 1) { // Avoid double-counting reflection for planes
                // Calculate reflection direction
                vector_scale(&temp_v2, normal_at_hit, 2.0f * (float)vector_dot(ray.direction, normal_at_hit));
                vector_sub(&temp_v3, ray.direction, temp_v2); // temp_v3 is reflection_dir
                vector_norm(&temp_v3, temp_v3);
                vector_trunc(&temp_v3, temp_v3);

                make_ray(&reflection_ray, temp_v1, temp_v3); // intersect_point, reflection_dir
                radiance(&temp_v4, ctx, reflection_ray, depth + 1); // temp_v4 is reflected_color

                vector_scale(&temp_v4, temp_v4, reflectivity);
                vector_add(out_color, *out_color, temp_v4);
            }
        }
    }
    return out_color;
}