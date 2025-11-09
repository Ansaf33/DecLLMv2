#include <stdio.h>    // For printf in main and mock cpu_interrupt
#include <stdint.h>   // For fixed-width integer types
#include <string.h>   // For memset in main

// Type definitions to match the original snippet's intent
typedef uint8_t byte;
typedef uint32_t uint;
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;

// CONCAT11 macro: Combines two 1-byte values into a 2-byte value.
// The first argument becomes the most significant byte, the second the least significant.
#define CONCAT11(high, low) ((undefined2)(((byte)(high)) << 8 | ((byte)(low))))

// --- Emulator Memory and State ---
// Global emulated memory block (e.g., Game Boy RAM + registers)
// Size 0x10000 (64KB) is typical for Game Boy.
uint8_t emulated_memory[0x10000];

// Buffers for screen rendering.
// bg_buffer: Represents the full 256x256 pixel background map.
// screen_buffer: Represents the visible 160x144 pixel screen.
uint8_t bg_buffer[256 * 256];
uint8_t screen_buffer[160 * 144];

// --- Mock CPU Interrupt Function ---
// This function is called by lcd_tick but its actual implementation is outside the scope
// of the provided snippet. A mock is provided for compilation.
void cpu_interrupt(int *emu_state, int interrupt_type) {
    // In a real emulator, this would set a CPU interrupt flag.
    // printf("CPU Interrupt requested: type %d\n", interrupt_type);
}

// --- Original Functions (fixed and refactored) ---

// Function: bg_tile
// param_1: Pointer to the emulator state array (emu_state)
// param_2: Tile index
// Returns the memory address of the specified tile's data.
int bg_tile(int *emu_state, byte tile_index) {
  // Determine tile data address region based on LCDC register (0xFF40) bit 4.
  // If bit 4 is 0, tile data is at 0x9000-0x97FF (signed tile indexing).
  // If bit 4 is 1, tile data is at 0x8000-0x8FFF (unsigned tile indexing).
  if ((emulated_memory[emu_state[0] + 0xff40] & 0x10) == 0) {
    // Signed tile indexing: tile_index (0-255) is treated as signed char (-128 to 127).
    // This maps tile numbers 0x80-0xFF to 0x880-0x8FF offsets, and 0x00-0x7F to 0x900-0x97F offsets.
    return emu_state[0] + (((char)tile_index + 0x900) * 0x10);
  } else {
    // Unsigned tile indexing: tile_index (0-255) is treated as unsigned.
    // This maps tile numbers 0x00-0xFF to 0x800-0x8FF offsets.
    return emu_state[0] + ((uint)tile_index + 0x800) * 0x10;
  }
}

// Function: draw_tile
// palette_base_addr: Base address in emulated_memory where palette data is stored.
// screen_dest_addr: Base address in the screen buffer to draw to.
// screen_pitch: Width of the screen buffer in pixels (e.g., 256 for background).
// tile_data_addr: Memory address where the 8x8 tile's pixel data is located.
// param_5: An additional parameter, often used for sprite flags (0 for background).
void draw_tile(int palette_base_addr, int screen_dest_addr, int screen_pitch, int tile_data_addr, int param_5) {
  for (byte row_offset = 0; row_offset < 8; row_offset++) {
    // Read the two bytes of pixel data for the current tile row.
    // Each pixel is 2 bits, so two bytes store data for 8 pixels.
    uint8_t tile_byte_low = emulated_memory[tile_data_addr + (uint)row_offset * 2];
    uint8_t tile_byte_high = emulated_memory[tile_data_addr + (uint)row_offset * 2 + 1];
    
    for (byte col_offset = 0; col_offset < 8; col_offset++) {
      // Calculate the bit mask to extract the pixel data for the current column.
      // Pixels are ordered from left (bit 7) to right (bit 0).
      uint bit_mask = 1 << (7 - col_offset);
      
      // Determine the 2-bit pixel color index (0-3).
      // Bit 1 comes from tile_byte_high, Bit 0 from tile_byte_low.
      uint pixel_color_index = 0;
      if ((bit_mask & tile_byte_high) != 0) {
        pixel_color_index = 2; // Set bit 1
      }
      if ((bit_mask & tile_byte_low) != 0) {
        pixel_color_index |= 1; // Set bit 0
      }
      
      // If param_5 is 0 (typically for background tiles), render the pixel.
      if (param_5 == 0) {
        // Calculate the destination address in the screen buffer.
        // screen_dest_addr + (row_offset * screen_pitch) + col_offset
        // Fetch the actual color value from the palette (stored in emulated_memory).
        // (param_5 + 8) * 4 evaluates to 32 when param_5 is 0.
        // This offset is used to access the correct palette for background (BGP).
        emulated_memory[screen_dest_addr + (uint)row_offset * screen_pitch + (uint)col_offset] =
             emulated_memory[palette_base_addr + (param_5 + 8) * 4 + pixel_color_index];
      }
    }
  }
}

// Function: update_bg
// param_1: Pointer to the emulator state array (emu_state)
// Renders the entire background layer into the bg_buffer.
void update_bg(int *emu_state) {
  int bg_map_base_addr;
  
  // Determine the background map base address based on LCDC register (0xFF40) bit 3.
  // If bit 3 is 0, map is at 0x9800-0x9BFF.
  // If bit 3 is 1, map is at 0x9C00-0x9FFF.
  if ((emulated_memory[emu_state[0] + 0xff40] & 8) == 0) {
    bg_map_base_addr = emu_state[0] + 0x9800;
  } else {
    bg_map_base_addr = emu_state[0] + 0x9c00;
  }

  // Iterate over the 32x32 tiles of the background map.
  for (byte row = 0; row < 0x20; row++) { // 32 rows
    for (byte col = 0; col < 0x20; col++) { // 32 columns
      // Read the tile index from the background map.
      byte tile_index = emulated_memory[bg_map_base_addr + col + row * 0x20];
      
      // Calculate the screen buffer destination for this tile.
      // emu_state[6] points to bg_buffer.
      // CONCAT11(row, col) creates (row << 8 | col). Multiplying by 8 gives (row * 2048 + col * 8),
      // which is equivalent to (row * 8 * 256 + col * 8) for an 8x8 tile on a 256-pixel pitch.
      draw_tile(emu_state[0], // Base address for palettes (emulated_memory)
                emu_state[6] + (uint)CONCAT11(row, col) * 8, // Destination in bg_buffer
                0x100, // Screen pitch (256 pixels)
                bg_tile(emu_state, tile_index), // Address of tile data
                0); // param_5 = 0 for background
    }
  }
}

// Function: update_screen
// param_1: Pointer to the emulator state array (emu_state)
// Updates the visible screen buffer for the current scanline.
void update_screen(int *emu_state) {
  unsigned char palette_register_value; // Temporary for register values
  char scroll_x_value;                  // Temporary for register values (SCX)
  undefined2 current_line_vram_address; // Composed VRAM address for current scanline
  
  // Update palette data from hardware registers (BGP, OBP0, OBP1 at 0xFF47-0xFF49).
  // These registers contain 2-bit color indices packed into a byte.
  for (int i = 0; i < 3; i++) {
    palette_register_value = emulated_memory[emu_state[0] + i + 0xff47];
    
    // Unpack the 4 color indices (2 bits each) and store them sequentially
    // into the emulator's palette array (starting at emu_state[8]).
    // This directly targets bytes within the int array.
    ((unsigned char*)&emu_state[8])[i * 4] = palette_register_value & 3;
    ((unsigned char*)&emu_state[8])[i * 4 + 1] = (palette_register_value >> 2) & 3;
    ((unsigned char*)&emu_state[8])[i * 4 + 2] = (palette_register_value >> 4) & 3;
    ((unsigned char*)&emu_state[8])[i * 4 + 3] = palette_register_value >> 6;
  }
  
  // Read the current scanline (LY) register.
  palette_register_value = emulated_memory[emu_state[0] + 0xff44];
  
  // Check if Background/Window Display is enabled (LCDC register 0xFF40, bit 0).
  if ((emulated_memory[emu_state[0] + 0xff40] & 1) != 0) {
    update_bg(emu_state); // Render the entire background layer.
    
    // Read the SCX (Scroll X) register.
    scroll_x_value = (char)emulated_memory[emu_state[0] + 0xff43];
    
    // Calculate the high byte of the VRAM address for the current scanline.
    // This combines LY (current scanline) and SCY (Scroll Y) values.
    current_line_vram_address = (undefined2)(((byte)(palette_register_value + (char)emulated_memory[emu_state[0] + 0xff42])) << 8);
    
    // Copy pixels for the current scanline from the background buffer to the final screen buffer.
    for (byte pixel_x_offset = 0; pixel_x_offset < 0xa0; pixel_x_offset++) { // 160 pixels wide
      // Update the low byte of the VRAM address using pixel_x_offset and SCX.
      current_line_vram_address = CONCAT11((current_line_vram_address >> 8), pixel_x_offset + scroll_x_value);
      
      // Copy the pixel data:
      // Destination: emu_state[7] (screen_buffer) + (LY * 160) + pixel_x_offset
      // Source:      emu_state[6] (bg_buffer) + current_line_vram_address (calculated VRAM address)
      emulated_memory[emu_state[7] + pixel_x_offset + (uint)palette_register_value * 0xa0] =
           emulated_memory[emu_state[6] + current_line_vram_address];
    }
  }
}

// Function: lcd_tick
// param_1: Pointer to the emulator state array (emu_state)
// Simulates one LCD clock tick, updating internal state and registers.
// Returns 1 (undefined4 type suggests it might return more complex data, but original always returns 1).
undefined4 lcd_tick(int *emu_state) {
  uint8_t current_ly; // Local variable for current scanline (LY)
  
  emu_state[0x14]++; // Increment internal LCD tick counter (param_1[20])
  
  // Check if the tick counter has reached the end of a scanline's cycle (0x6d = 109 cycles).
  if (emu_state[0x14] == 0x6d) {
    emu_state[0x14] = 0; // Reset tick counter for the new scanline.
  }

  // This block handles events that occur at the start of a new scanline's processing (tick counter is 0).
  if (emu_state[0x14] == 0) {
    current_ly = emulated_memory[emu_state[0] + 0xff44]; // Read current LY register.
    
    if (current_ly < 0x90) { // If current LY is within the visible screen area (0-143).
      emulated_memory[emu_state[0] + 0xff44]++; // Increment LY register.
      current_ly++; // Update local variable for consistency.
      
      if (current_ly == 0x90) { // If LY just reached 144 (start of VBLANK period).
        emu_state[0x11] = 1; // Set VBLANK flag in emulator state (param_1[17]).
        
        // Set LCD mode to 1 (VBLANK) in STAT register (0xFF41).
        emulated_memory[emu_state[0] + 0xff41] = (emulated_memory[emu_state[0] + 0xff41] & 0xfc) | 1;
        
        // Check if VBLANK interrupt is enabled (STAT bit 4).
        if ((emulated_memory[emu_state[0] + 0xff41] & 0x10) != 0) {
          cpu_interrupt(emu_state, 1);
        }
        cpu_interrupt(emu_state, 0); // VBLANK interrupt (or some other type 0).
        return 1; // Event handled, return.
      }
    } else { // If current LY is in the VBLANK area (144-153).
      emulated_memory[emu_state[0] + 0xff44]++; // Increment LY register.
      current_ly++; // Update local variable.
      
      if (current_ly == 0x9a) { // If LY just reached 154 (end of VBLANK period).
        emulated_memory[emu_state[0] + 0xff44] = 0; // Reset LY to 0.
        // Execution falls through to the final block to set mode 2 for the new scanline (LY=0).
      }
      // If LY is between 0x90 and 0x99 (exclusive of 0x9A), execution also falls through.
      // This means the final block will handle setting the mode for the next visible scanline.
    }
  } else { // If not at the start of a scanline's cycle (emu_state[0x14] != 0).
    current_ly = emulated_memory[emu_state[0] + 0xff44]; // Read current LY.
    
    if (current_ly < 0x90) { // Only process mode changes for visible lines (0-143).
      if (emu_state[0x14] == 0x13) { // At 0x13 (19) cycles into the scanline (OAM Scan period).
        // Set LCD mode to 3 (Drawing VRAM).
        emulated_memory[emu_state[0] + 0xff41] = (emulated_memory[emu_state[0] + 0xff41] & 0xfc) | 3;
        return 1; // Event handled, return.
      }
      if (emu_state[0x14] == 0x3c) { // At 0x3c (60) cycles into the scanline (HBLANK period).
        // Set LCD mode to 0 (HBLANK).
        emulated_memory[emu_state[0] + 0xff41] &= 0xfc; // Clear mode bits (set to 0).
        
        // Check if OAM interrupt is enabled (STAT bit 3).
        if ((emulated_memory[emu_state[0] + 0xff41] & 8) != 0) {
          cpu_interrupt(emu_state, 1);
        }
        return 1; // Event handled, return.
      }
    }
    // If none of the specific tick points (0, 0x13, 0x3c) were met,
    // and it's not the end of VBLANK (LY=0x9A), then simply return.
    // This handles the intermediate cycles within OAM scan, VRAM draw, and HBLANK.
    return 1;
  }

  // This final block is reached if emu_state[0x14] was 0, AND:
  // 1. LY was < 0x90 and did not become 0x90 (i.e., LY is now 1-143).
  // 2. LY was >= 0x90 and became 0x9A, resetting to 0 (i.e., LY is now 0).
  // In both cases, this indicates the start of a visible scanline (LY is 0-143).
  
  // Set LCD mode to 2 (OAM Scan).
  emulated_memory[emu_state[0] + 0xff41] = (emulated_memory[emu_state[0] + 0xff41] & 0xfc) | 2;
  
  // Check if OAM interrupt is enabled (STAT bit 5).
  if ((emulated_memory[emu_state[0] + 0xff41] & 0x20) != 0) {
    cpu_interrupt(emu_state, 1);
  }
  
  // Compare LY and LYC registers (LY Compare).
  current_ly = emulated_memory[emu_state[0] + 0xff44]; // Re-read LY to ensure it's up-to-date.
  emulated_memory[emu_state[0] + 0xff41] &= 0xfb; // Clear LYC=LY flag (STAT bit 2).
  if (current_ly == emulated_memory[emu_state[0] + 0xff45]) {
    emulated_memory[emu_state[0] + 0xff41] |= 4; // Set LYC=LY flag.
    // Check if LYC=LY interrupt is enabled (STAT bit 6).
    if ((emulated_memory[emu_state[0] + 0xff41] & 0x40) != 0) {
      cpu_interrupt(emu_state, 1);
    }
  }
  
  update_screen(emu_state); // Update the screen for the current scanline.
  return 1;
}

// --- Main Function (for Linux compilation) ---
int main() {
    // Initialize emulator state array.
    // Index 0x14 (20) is the highest used, so size 21 is needed.
    int emu_state[21];

    // emu_state[0] stores the base address of the emulated memory.
    emu_state[0] = (int)(intptr_t)emulated_memory;

    // Initialize emulated_memory to zeros.
    memset(emulated_memory, 0, sizeof(emulated_memory));

    // Set up some initial register values for basic LCD operation.
    // LCDC (0xFF40): Bit 7 (LCD Enable), Bit 0 (BG/Window Display Enable)
    emulated_memory[emu_state[0] + 0xff40] = 0x91; 
    // STAT (0xFF41): Mode 0 (HBLANK)
    emulated_memory[emu_state[0] + 0xff41] = 0x00; 
    // LY (0xFF44): Current scanline, starts at 0.
    emulated_memory[emu_state[0] + 0xff44] = 0x00; 
    // LYC (0xFF45): LY Compare register, set to 0 for this example.
    emulated_memory[emu_state[0] + 0xff45] = 0x00; 
    // BGP (0xFF47): Background Palette. 0xE4 (11100100) sets colors 0-3 to 0,1,2,3 respectively.
    emulated_memory[emu_state[0] + 0xff47] = 0xE4; 
    // SCY (0xFF42) and SCX (0xFF43)
    emulated_memory[emu_state[0] + 0xff42] = 0x00; // SCY = 0
    emulated_memory[emu_state[0] + 0xff43] = 0x00; // SCX = 0

    // emu_state[6] and emu_state[7] point to the rendering buffers.
    emu_state[6] = (int)(intptr_t)bg_buffer;
    emu_state[7] = (int)(intptr_t)screen_buffer;

    // Initialize internal emulator state variables.
    emu_state[0x11] = 0; // VBLANK flag.
    emu_state[0x14] = 0; // LCD tick counter.

    // --- Populate some fake tile data and background map for a visual test ---
    // Tile 0: A solid color tile (e.g., color index 0x03 for all pixels).
    // Tile data is 16 bytes per 8x8 tile.
    for (int i = 0; i < 16; ++i) {
        emulated_memory[emu_state[0] + 0x8000 + i] = 0xFF; // All bits set for both planes -> color 3.
    }
    // Background map (0x9800-0x9BFF): Fill with tile 0.
    // 32x32 tiles, each entry is 1 byte.
    for (int i = 0; i < 0x400; ++i) { // 0x400 = 1024 bytes (32*32)
        emulated_memory[emu_state[0] + 0x9800 + i] = 0; // All tiles are Tile 0.
    }

    // --- Simulation Loop ---
    printf("Starting LCD simulation for 10 frames...\n");
    for (int frame = 0; frame < 10; ++frame) {
        // A full Game Boy frame is 154 scanlines * 109 cycles/scanline = 16786 cycles.
        for (int tick = 0; tick < 16786; ++tick) {
            lcd_tick(emu_state);
        }
        printf("Frame %d completed. Current LY: %d, VBLANK flag: %d\n", 
               frame, emulated_memory[emu_state[0] + 0xff44], emu_state[0x11]);
        emu_state[0x11] = 0; // Clear VBLANK flag for next frame.
        
        // In a real emulator, here you would blit screen_buffer to an actual display.
        // For this example, we just print a message.
    }
    printf("Simulation finished.\n");

    return 0;
}