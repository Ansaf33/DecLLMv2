#include <stdint.h> // For uint16_t, uint32_t, uint8_t, uintptr_t
#include <string.h> // For memset
#include <stdlib.h> // For malloc, free, exit
#include <stdio.h>  // For fprintf, stderr

// --- Custom function declarations/definitions to make the code compilable ---
// These simulate the original environment's functions.

// Allocate memory, similar to malloc. Returns NULL on failure.
void* allocate(size_t size) {
    return malloc(size);
}

// Deallocate memory, similar to free.
void deallocate(void* ptr, size_t size) {
    // The original deallocate takes a size argument.
    // Standard free() doesn't need size. This size might be for tracking or custom allocators.
    // For this example, we'll just call free().
    (void)size; // Suppress unused parameter warning
    free(ptr);
}

// Error termination function. Assumed to exit the program.
void _terminate(void) {
    fprintf(stderr, "Fatal error: _terminate() called.\n");
    exit(1);
}

// Placeholder for GetColorIndex.
// In a real implementation, this would access the layer data of the canvas.
uint32_t GetColorIndex(uint16_t* canvas_ptr, uint16_t x, uint16_t y, uint8_t layer) {
    (void)canvas_ptr; (void)x; (void)y; (void)layer;
    // Example: return a specific color for a pixel on layer 1, otherwise transparent.
    if (layer == 1 && x == 5 && y == 5) return 0x01;
    return 0x14; // Default transparent color
}

// Placeholder for SetColor.
// In a real implementation, this would write to the layer data of the canvas.
void SetColor(uint16_t* canvas_ptr, uint16_t x, uint16_t y, uint8_t layer, uint8_t color_index) {
    (void)canvas_ptr; (void)x; (void)y; (void)layer; (void)color_index;
    // Example: printf("SetColor: x=%u, y=%u, layer=%u, color=%u\n", x, y, layer, color_index);
}
// --- End of custom function declarations/definitions ---

// Max layers constraint from `param_4 < 7` (meaning param_4 can be 0-6)
#define MAX_CANVAS_LAYERS_COUNT 7
// Fixed header size as inferred from the original code (0x60 bytes)
#define CANVAS_HEADER_SIZE 0x60
// Default transparent color value used for initializing layers
#define DEFAULT_TRANSPARENT_COLOR 0x14

// Function: CreateCanvas
// Allocates and initializes a new canvas structure.
// param_1: A pointer to a uint16_t* where the newly created canvas pointer will be stored.
// param_2: The width of the canvas.
// param_3: The height of the canvas.
// param_4: The number of layers for the canvas (0-6).
// Returns 0 on success, 0xFFFFFFFF on failure (e.g., too many layers).
uint32_t CreateCanvas(uint16_t **canvas_out, uint16_t width, uint16_t height, uint8_t num_layers) {
    if (num_layers >= MAX_CANVAS_LAYERS_COUNT) {
        return 0xFFFFFFFF; // Error: number of layers exceeds maximum allowed.
    }

    // Allocate the canvas header (fixed size 0x60 bytes).
    uint16_t* canvas_ptr = (uint16_t*)allocate(CANVAS_HEADER_SIZE);
    if (canvas_ptr == NULL) {
        _terminate(); // Fatal error: memory allocation failed.
        return 0xFFFFFFFF; // Unreachable if _terminate() exits.
    }

    // Initialize the entire header memory to 0.
    memset(canvas_ptr, 0, CANVAS_HEADER_SIZE);

    // Use char* for byte-level access to safely handle mixed data types within the header
    // (uint16_t for dimensions, uint8_t for layer count, uint32_t for layer pointers).
    char* canvas_bytes = (char*)canvas_ptr;

    // Store width, height, and number of layers in the header.
    *(uint16_t*)canvas_bytes = width;           // Offset 0: width
    *(uint16_t*)(canvas_bytes + 2) = height;    // Offset 2: height
    *(uint8_t*)(canvas_bytes + 4) = num_layers; // Offset 4: number of layers

    // Layer pointers are stored as an array of uint32_t starting at offset 8.
    uint32_t* layer_pointers = (uint32_t*)(canvas_bytes + 8);
    uint32_t layer_data_size = (uint32_t)width * height; // Size of each layer (pixels, assuming 1 byte per pixel).

    // Allocate and initialize memory for each layer.
    for (uint8_t i = 0; i < num_layers; ++i) {
        uint8_t* layer_data = (uint8_t*)allocate(layer_data_size);
        if (layer_data == NULL) {
            // Memory allocation for a layer failed. Clean up previously allocated resources.
            for (uint8_t j = 0; j < i; ++j) {
                deallocate((void*)(uintptr_t)layer_pointers[j], layer_data_size);
            }
            deallocate(canvas_ptr, CANVAS_HEADER_SIZE);
            _terminate(); // Fatal error, program exits.
            return 0xFFFFFFFF; // Unreachable.
        }
        // Store the layer data pointer (cast to uint32_t) in the header.
        layer_pointers[i] = (uint32_t)(uintptr_t)layer_data;
        // Initialize the layer data with a default transparent color.
        memset(layer_data, DEFAULT_TRANSPARENT_COLOR, layer_data_size);
    }

    *canvas_out = canvas_ptr; // Pass the pointer to the newly created canvas back to the caller.
    return 0; // Success.
}

// Function: DestroyCanvas
// Deallocates all memory associated with a canvas.
// param_1: A pointer to the canvas handle (uint16_t**), which will be set to NULL after destruction.
void DestroyCanvas(uint16_t **canvas_handle) {
    if (canvas_handle == NULL || *canvas_handle == NULL) {
        return; // Nothing to destroy.
    }

    uint16_t* canvas_ptr = *canvas_handle;
    char* canvas_bytes = (char*)canvas_ptr;

    // Retrieve canvas dimensions and layer count from the header.
    uint16_t width = *(uint16_t*)canvas_bytes;
    uint16_t height = *(uint16_t*)(canvas_bytes + 2);
    uint8_t num_layers = *(uint8_t*)(canvas_bytes + 4);

    // Get the array of layer pointers.
    uint32_t* layer_pointers = (uint32_t*)(canvas_bytes + 8);
    uint32_t layer_data_size = (uint32_t)width * height;

    // Deallocate memory for each layer.
    for (uint8_t i = 0; i < num_layers; ++i) {
        if (layer_pointers[i] != 0) { // Check if the layer pointer is valid (not NULL).
            deallocate((void*)(uintptr_t)layer_pointers[i], layer_data_size);
            layer_pointers[i] = 0; // Clear the stored pointer after deallocation.
        }
    }

    // Deallocate the canvas header itself.
    deallocate(canvas_ptr, CANVAS_HEADER_SIZE);
    *canvas_handle = NULL; // Clear the caller's canvas handle to prevent dangling pointers.
}

// Function: FlattenCanvas
// Merges all layers (from layer 1 onwards) into layer 0 of the canvas.
// Transparent pixels (0x14) are ignored during merging.
// param_1: A pointer to the canvas structure.
void FlattenCanvas(uint16_t *canvas_ptr) {
    if (canvas_ptr == NULL) {
        return; // Nothing to flatten.
    }

    char* canvas_bytes = (char*)canvas_ptr;

    // Retrieve canvas dimensions and layer count from the header.
    uint16_t width = *(uint16_t*)canvas_bytes;
    uint16_t height = *(uint16_t*)(canvas_bytes + 2);
    uint8_t num_layers = *(uint8_t*)(canvas_bytes + 4);

    // Iterate through layers starting from 1 (the second layer) up to num_layers-1.
    // Layer 0 is the target for flattening.
    for (uint8_t layer_idx = 1; layer_idx < num_layers; ++layer_idx) {
        for (uint16_t y = 0; y < height; ++y) {
            for (uint16_t x = 0; x < width; ++x) {
                uint32_t color_index = GetColorIndex(canvas_ptr, x, y, layer_idx);
                // If the color is not the default transparent color, apply it to the base layer (layer 0).
                if (color_index != DEFAULT_TRANSPARENT_COLOR) {
                    SetColor(canvas_ptr, x, y, 0, (uint8_t)color_index);
                }
            }
        }
    }
}