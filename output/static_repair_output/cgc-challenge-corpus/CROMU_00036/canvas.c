#include <stdio.h>  // For printf in main
#include <stdlib.h> // For malloc, free
#include <string.h> // For memset
#include <stdint.h> // For uint16_t, uint8_t, uint32_t, uintptr_t

// Forward declarations for functions not provided in the snippet
// Assuming GetColorIndex and SetColor operate on uint8_t values for colors
uint32_t GetColorIndex(uint16_t *canvas_base, uint16_t x, uint16_t y, uint8_t layer_idx);
void SetColor(uint16_t *canvas_base, uint16_t x, uint16_t y, uint8_t layer_idx, uint8_t color_index);

// Helper function to deallocate layers in case of an error during CreateCanvas or for DestroyCanvas
static void cleanup_canvas_layers(uint16_t *canvas_base, uint8_t allocated_layers_count, uint16_t width, uint16_t height) {
    if (!canvas_base) {
        return;
    }

    // The size of each layer is width * height * sizeof(uint8_t)
    size_t layer_data_size = (size_t)width * height;
    if (layer_data_size == 0 && width != 0 && height != 0) {
        // Handle potential overflow or invalid dimensions if needed, though unlikely for ushort
    }

    // Layer pointers are stored starting at `canvas_base + 4` (ushort indices).
    // Each pointer is 4 bytes (uint32_t), occupying 2 ushort slots.
    for (int i = 0; i < allocated_layers_count; ++i) {
        // Calculate the address of the uint32_t slot where the layer pointer is stored
        uint32_t *layer_data_ptr_slot = (uint32_t*)((uintptr_t)canvas_base + (4 + (size_t)i * 2) * sizeof(uint16_t));
        if (*layer_data_ptr_slot != 0) { // Check if layer was actually allocated
            free((void*)(uintptr_t)*layer_data_ptr_slot);
            *layer_data_ptr_slot = 0; // Clear the pointer in the canvas structure
        }
    }
}

// Function: CreateCanvas
int CreateCanvas(uint16_t **canvas_ptr_out, uint16_t width, uint16_t height, uint8_t num_layers) {
    // Original condition: param_4 < 7 (max 6 layers)
    if (num_layers >= 7) {
        return -1; // Error code
    }

    // Allocate the base canvas structure (96 bytes = 0x60)
    // This structure holds width, height, num_layers, and pointers to layer data.
    uint16_t *canvas_base = (uint16_t *)malloc(0x60);
    if (canvas_base == NULL) {
        return -1; // Allocation failed
    }
    memset(canvas_base, 0, 0x60); // Initialize all to zero

    // Store canvas metadata
    canvas_base[0] = width;
    canvas_base[1] = height;
    // Store num_layers at the byte address corresponding to `(uint8_t*)canvas_base + 4`.
    ((uint8_t*)canvas_base)[4] = num_layers;

    // Calculate size for each layer's pixel data. Each pixel is a uint8_t color index.
    size_t layer_data_size = (size_t)width * height;
    if (layer_data_size == 0 && width != 0 && height != 0) {
         free(canvas_base);
         return -1; // Prevent potential zero-size allocation issues if width or height are large but product is 0
    }

    // Allocate memory for each layer
    for (int i = 0; i < num_layers; ++i) {
        uint8_t *layer_data = (uint8_t *)malloc(layer_data_size);
        if (layer_data == NULL) {
            // Cleanup previously allocated layers and the base canvas structure
            cleanup_canvas_layers(canvas_base, i, width, height); // `i` is the count of successfully allocated layers
            free(canvas_base);
            return -1; // Allocation failed
        }
        // Fill the layer with 0x14 (20) as per original code
        memset(layer_data, 0x14, layer_data_size);

        // Store the pointer to the allocated layer data
        // Pointers are stored as uint32_t starting from `canvas_base + 4` (ushort indices).
        // Each uint32_t pointer occupies 2 ushorts.
        uint32_t *layer_ptr_slot = (uint32_t*)((uintptr_t)canvas_base + (4 + (size_t)i * 2) * sizeof(uint16_t));
        *layer_ptr_slot = (uint32_t)(uintptr_t)layer_data; // Store pointer as uint32_t
    }

    *canvas_ptr_out = canvas_base; // Return the base canvas pointer
    return 0; // Success
}

// Function: DestroyCanvas
void DestroyCanvas(uint16_t **canvas_ptr_out) {
    uint16_t *canvas_base = *canvas_ptr_out;

    if (canvas_base != NULL) {
        uint16_t width = canvas_base[0];
        uint16_t height = canvas_base[1];
        uint8_t num_layers = ((uint8_t*)canvas_base)[4]; // Get num_layers from its stored byte location

        // Deallocate each layer's pixel data
        cleanup_canvas_layers(canvas_base, num_layers, width, height);

        // Deallocate the base canvas structure itself
        free(canvas_base);
        *canvas_ptr_out = NULL; // Clear the pointer
    }
}

// Function: FlattenCanvas
void FlattenCanvas(uint16_t *canvas_base) {
    if (canvas_base == NULL) {
        return; // Handle NULL input
    }

    uint16_t width = canvas_base[0];
    uint16_t height = canvas_base[1];
    uint8_t num_layers = ((uint8_t*)canvas_base)[4]; // Get num_layers

    // Loop through layers, starting from the second layer (index 1)
    for (uint8_t layer_idx = 1; layer_idx < num_layers; ++layer_idx) {
        for (uint16_t x = 0; x < width; ++x) {
            for (uint16_t y = 0; y < height; ++y) {
                // Get the color index from the current layer
                uint32_t color_index = GetColorIndex(canvas_base, x, y, layer_idx);

                // If the color index is not 0x14 (transparent/empty), transfer it to the base layer (layer 0)
                if (color_index != 0x14) {
                    SetColor(canvas_base, x, y, 0, (uint8_t)color_index);
                }
            }
        }
    }
}

// Dummy implementation for GetColorIndex
// This function retrieves the color index at (x, y) on a specific layer.
uint32_t GetColorIndex(uint16_t *canvas_base, uint16_t x, uint16_t y, uint8_t layer_idx) {
    if (canvas_base == NULL || x >= canvas_base[0] || y >= canvas_base[1] || layer_idx >= ((uint8_t*)canvas_base)[4]) {
        return 0x14; // Return default transparent color for out-of-bounds access
    }

    // Get the pointer to the layer data
    uint32_t *layer_ptr_slot = (uint32_t*)((uintptr_t)canvas_base + (4 + (size_t)layer_idx * 2) * sizeof(uint16_t));
    uint8_t *layer_data = (uint8_t*)(uintptr_t)*layer_ptr_slot;

    if (layer_data == NULL) {
        return 0x14; // Layer not allocated, return default transparent color
    }

    // Calculate 1D index within the layer data array
    size_t index = (size_t)y * canvas_base[0] + x;
    return layer_data[index];
}

// Dummy implementation for SetColor
// This function sets the color index at (x, y) on a specific layer.
void SetColor(uint16_t *canvas_base, uint16_t x, uint16_t y, uint8_t layer_idx, uint8_t color_index) {
    if (canvas_base == NULL || x >= canvas_base[0] || y >= canvas_base[1] || layer_idx >= ((uint8_t*)canvas_base)[4]) {
        return; // Out-of-bounds access, do nothing
    }

    // Get the pointer to the layer data
    uint32_t *layer_ptr_slot = (uint32_t*)((uintptr_t)canvas_base + (4 + (size_t)layer_idx * 2) * sizeof(uint16_t));
    uint8_t *layer_data = (uint8_t*)(uintptr_t)*layer_ptr_slot;

    if (layer_data == NULL) {
        return; // Layer not allocated, do nothing
    }

    // Calculate 1D index within the layer data array
    size_t index = (size_t)y * canvas_base[0] + x;
    layer_data[index] = color_index;
}

// Example main function to demonstrate usage
int main() {
    uint16_t *my_canvas = NULL;
    int result = CreateCanvas(&my_canvas, 10, 10, 3); // 10x10 canvas, 3 layers

    if (result == 0) {
        printf("Canvas created successfully.\n");

        // Set colors on different layers
        SetColor(my_canvas, 0, 0, 0, 1);     // Base layer, color 1
        SetColor(my_canvas, 0, 0, 1, 2);     // Layer 1, color 2
        SetColor(my_canvas, 0, 0, 2, 0x14);  // Layer 2, transparent (0x14)

        // Check colors before flatten
        printf("Before flatten: Layer 0 (0,0) = %u, Layer 1 (0,0) = %u, Layer 2 (0,0) = %u\n",
               GetColorIndex(my_canvas, 0, 0, 0),
               GetColorIndex(my_canvas, 0, 0, 1),
               GetColorIndex(my_canvas, 0, 0, 2));

        FlattenCanvas(my_canvas);

        // Check color on base layer after flatten
        printf("After flatten: Layer 0 (0,0) = %u\n", GetColorIndex(my_canvas, 0, 0, 0)); // Should be 2 from Layer 1

        DestroyCanvas(&my_canvas);
        printf("Canvas destroyed.\n");
    } else {
        printf("Failed to create canvas.\n");
    }

    // Test error case: too many layers
    result = CreateCanvas(&my_canvas, 10, 10, 7);
    if (result != 0) {
        printf("Successfully failed to create canvas with too many layers (expected).\n");
    }

    return 0;
}