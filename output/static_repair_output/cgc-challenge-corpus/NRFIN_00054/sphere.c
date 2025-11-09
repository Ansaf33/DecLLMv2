#include <math.h>   // For fabs
#include <stdio.h>  // For printf

// Define the Sphere struct based on the observed offsets in the original snippet.
// These offsets (0x54, 0x58, 0x5c) suggest fields within a larger structure.
typedef struct Sphere {
    // Padding to ensure 'some_int_field' starts at offset 0x54 bytes from the struct's beginning.
    char _padding_to_54[0x54]; 
    unsigned int some_int_field; // Corresponds to *(undefined4 *)(param_1 + 0x54)
    float radius;                // Corresponds to *(float *)(param_1 + 0x58)
    void (*intersect_func)(void*); // Corresponds to *(code **)(param_1 + 0x5c)
} Sphere;

// Dummy function for sphere_intersect, as it was referenced but not defined.
// It takes a void* to represent the sphere object it might operate on.
void sphere_intersect(void* sphere_ptr) {
    printf("Sphere intersect function called for object at %p.\n", sphere_ptr);
    // In a real application, this function would perform actual intersection calculations.
}

// Function: sphere_init
// Original signature: void sphere_init(int param_1, undefined4 param_2, undefined4 param_3)
//
// Interpretation and Fixes:
// 1. `param_1` is interpreted as a pointer to a `Sphere` object (`Sphere* s`).
// 2. `CONCAT44(param_3, param_2)` is a decompiler artifact, likely representing a `double` value
//    that was split into two 32-bit components (`param_2`, `param_3`). To simplify and reduce
//    intermediate variables, the function signature is changed to directly accept a `double`
//    for the `initial_radius`. This eliminates the need for `CONCAT44` and the `dVar1` variable.
void sphere_init(Sphere* s, double initial_radius) {
  // Corresponds to: *(undefined4 *)(param_1 + 0x54) = 0;
  s->some_int_field = 0;

  // Corresponds to: dVar1 = fabs((double)CONCAT44(param_3,param_2));
  //                 *(float *)(param_1 + 0x58) = (float)dVar1;
  // By passing `initial_radius` directly, we avoid the intermediate `dVar1` and `CONCAT44`.
  s->radius = (float)fabs(initial_radius);

  // Corresponds to: *(code **)(param_1 + 0x5c) = sphere_intersect;
  s->intersect_func = sphere_intersect;
  return;
}

// Main function to demonstrate the usage of sphere_init and other components.
int main() {
    Sphere my_sphere; // Create a sphere object on the stack

    // Initialize the sphere with a negative radius to test fabs
    printf("Initializing my_sphere with radius -10.5...\n");
    sphere_init(&my_sphere, -10.5);

    // Print the initialized values to verify
    printf("my_sphere initialized values:\n");
    printf("  Field at offset 0x54: %u\n", my_sphere.some_int_field);
    printf("  Radius (at offset 0x58): %f (expected 10.5)\n", my_sphere.radius);
    printf("  Intersect function pointer (at offset 0x5c): %p\n", (void*)my_sphere.intersect_func);

    // Call the intersect function through the function pointer if it's set
    if (my_sphere.intersect_func) {
        printf("Calling intersect function for my_sphere...\n");
        my_sphere.intersect_func(&my_sphere);
    }

    printf("\n");

    Sphere another_sphere; // Create another sphere object
    // Initialize with a positive radius
    printf("Initializing another_sphere with radius 25.0...\n");
    sphere_init(&another_sphere, 25.0);

    printf("another_sphere initialized values:\n");
    printf("  Field at offset 0x54: %u\n", another_sphere.some_int_field);
    printf("  Radius (at offset 0x58): %f (expected 25.0)\n", another_sphere.radius);
    printf("  Intersect function pointer (at offset 0x5c): %p\n", (void*)another_sphere.intersect_func);

    // Call the intersect function for another_sphere
    if (another_sphere.intersect_func) {
        printf("Calling intersect function for another_sphere...\n");
        another_sphere.intersect_func(&another_sphere);
    }

    return 0;
}