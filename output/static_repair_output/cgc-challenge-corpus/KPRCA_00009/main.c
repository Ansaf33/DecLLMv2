#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>     // For read, write, close, ssize_t
#include <sys/select.h> // For fd_set, timeval
#include <fcntl.h>      // For open, O_RDONLY

// Custom types from decompiler, mapped to standard C types
typedef unsigned char byte;

// Assume a dummy GBState struct based on observed offsets.
// This is a minimal representation to make pointer arithmetic type-safe
// and to match the decompiler's member access patterns.
// Actual GBState would be much larger and complex.
struct GBState {
    // screen_buffer_ptr is accessed via ((int*)gb_context)[7], which means it's at offset 7 * sizeof(int)
    char _padding_0_1b[0x1c]; // Padding up to offset 0x1c (7 * sizeof(int) assuming sizeof(int) == 4)
    char* screen_buffer_ptr;  // At offset 0x1c (28 bytes)
    
    char _padding_20_2b[0x2c - 0x20]; // Padding up to offset 0x2c
    byte joypad_state;        // At offset 0x2c
    unsigned short af;        // At offset 0x2e
    unsigned short bc;        // At offset 0x30
    unsigned short de;        // At offset 0x32
    unsigned short hl;        // At offset 0x34
    unsigned short sp;        // At offset 0x36
    unsigned short pc;        // At offset 0x38
    
    char _padding_3a_3f[0x40 - 0x3a]; // Padding up to offset 0x40
    float float_val_0x40;     // At offset 0x40
    int int_val_0x44;         // At offset 0x44 (e.g., interrupt flag)
    
    char _padding_48_ff3f[0xff40 - 0x48]; // Padding up to offset 0xff40
    char lcdc_reg;            // At offset 0xff40
    // No further members explicitly identified from the snippet after lcdc_reg.
};

// External Game Boy emulator function declarations
extern void* gb_new(void);
extern bool gb_load(void* gb_context, const byte* rom_data, size_t rom_size);
extern void gb_reset(void* gb_context);
extern bool gb_tick(void* gb_context);

// Assuming these are wrappers for read/write/select
// receive: fd, buf, count, actual_bytes_read_ptr
extern int receive(int fd, void* buf, int count, int* bytes_read_ptr);
// transmit: fd, buf, count
extern int transmit(int fd, const void* buf, size_t count);
// fdwait: nfds, readfds, writefds, timeout_struct, num_ready_ptr
extern int fdwait(int nfds, fd_set* readfds, fd_set* writefds, struct timeval* timeout, int* num_ready_ptr);


// Global string literals (from DAT_ addresses in original snippet)
const char DAT_000174b6[] = "  "; // 0 - empty space
const char DAT_000174b8[] = "░░"; // 1 - light shade
const char DAT_000174ba[] = "▒▒"; // 2 - medium shade
const char DAT_000174bc[] = "██"; // 3 - dark shade (or solid block)
const char DAT_000174be[] = "\x1b[H"; // ANSI escape code: Move cursor to home position
const char DAT_000174c2[] = "\n"; // Newline


// Function: usage
void usage(const char* program_name) {
  printf("Usage: %s rom-file\n", program_name);
}

// Function: recvall
// Reads exactly 'total_bytes' into 'buffer' from 'fd'.
// Returns true on success, false on error or EOF before reading all bytes.
bool recvall(int fd, void* buffer, size_t total_bytes) {
  int bytes_received_this_call;
  size_t bytes_remaining = total_bytes;
  byte* current_buffer_ptr = (byte*)buffer; // Use byte* for pointer arithmetic

  while (bytes_remaining > 0) {
    // The original 'receive' function takes an int* for bytes_read_ptr
    int receive_result = receive(fd, current_buffer_ptr, (int)bytes_remaining, &bytes_received_this_call);
    
    // Check for error (receive_result != 0 implies error based on original code)
    // or if no bytes were read (bytes_received_this_call == 0) which signifies EOF or non-blocking read with no data
    if (receive_result != 0 || bytes_received_this_call == 0) {
        break; // Error or EOF
    }
    
    current_buffer_ptr += bytes_received_this_call;
    bytes_remaining -= bytes_received_this_call;
  }
  return bytes_remaining == 0;
}

// Function: load_rom
// Loads ROM data from the specified file into the Game Boy context.
// Returns true on success, false on failure.
bool load_rom(struct GBState* gb_context, const char* rom_filename) {
  byte rom_buffer[32768]; // 32KB buffer for ROM data
  int rom_fd;
  bool success = false;

  rom_fd = open(rom_filename, O_RDONLY);
  if (rom_fd == -1) {
    perror("Error opening ROM file");
    return false;
  }

  // Attempt to read 32KB of ROM data
  // The original code implies fixed size of 0x8000 (32768)
  if (!recvall(rom_fd, rom_buffer, sizeof(rom_buffer))) {
    fprintf(stderr, "Error reading ROM file or file too small.\n");
    close(rom_fd);
    return false;
  }

  // Call gb_load with the context, ROM data, and size
  success = gb_load(gb_context, rom_buffer, sizeof(rom_buffer));

  close(rom_fd);
  return success;
}

// Function: print_reg
void print_reg(const struct GBState* gb_context) {
  // Use explicit casts to char* for byte-level pointer arithmetic
  // Then cast to unsigned short* to dereference
  printf("AF = %04X, BC = %04X, DE = %04X, HL = %04X\n",
         (unsigned int)*(unsigned short*)((char*)gb_context + 0x2e),
         (unsigned int)*(unsigned short*)((char*)gb_context + 0x30),
         (unsigned int)*(unsigned short*)((char*)gb_context + 0x32),
         (unsigned int)*(unsigned short*)((char*)gb_context + 0x34));
  printf("SP = %04X, PC = %04X\n",
         (unsigned int)*(unsigned short*)((char*)gb_context + 0x36),
         (unsigned int)*(unsigned short*)((char*)gb_context + 0x38));
}

// Function: block_6px_char
const char* block_6px_char(const byte *pixel_data, int stride) {
  // Calculate the average pixel value for a 2x3 block.
  // This averages 6 pixels (a 2x3 block starting at pixel_data)
  unsigned int avg_val = ((unsigned int)pixel_data[0] + (unsigned int)pixel_data[1] +
                          (unsigned int)pixel_data[stride] + (unsigned int)pixel_data[stride + 1] +
                          (unsigned int)pixel_data[stride * 2] + (unsigned int)pixel_data[stride * 2 + 1]) / 6;

  if (avg_val == 3) {
    return DAT_000174bc; // Dark shade
  }
  if (avg_val == 2) {
    return DAT_000174ba; // Medium shade
  }
  if (avg_val == 1) {
    return DAT_000174b8; // Light shade
  }
  // Default case for avg_val == 0 or any other value not explicitly handled
  return DAT_000174b6; // Empty space
}

// Function: draw_screen
void draw_screen(struct GBState* gb_context) {
  // Access lcdc_reg at offset 0xff40
  // If it's negative (char is signed), then draw
  if (gb_context->lcdc_reg < 0) { // lcdc_reg is at 0xff40 offset
    transmit(1, DAT_000174be, strlen(DAT_000174be)); // Move cursor home
    
    const byte* screen_data = (const byte*)gb_context->screen_buffer_ptr;
    
    // Loop through rows (0 to 0x8F, step 3) and columns (0 to 0x9F, step 2)
    // This draws a 144x160 screen in 2x3 blocks, effectively 48x80 character grid
    for (byte row = 0; row < 0x90; row += 3) { // 144 / 3 = 48 rows of characters
      for (byte col = 0; col < 0xa0; col += 2) { // 160 / 2 = 80 columns of characters
        // Calculate the starting pixel for the 2x3 block
        const byte* current_pixel_block = screen_data + (row * 0xa0 + col);
        const char* block_char = block_6px_char(current_pixel_block, 0xa0); // 0xa0 is the stride (width of screen)
        transmit(1, block_char, strlen(block_char));
      }
      transmit(1, DAT_000174c2, strlen(DAT_000174c2)); // Newline after each row of characters
    }
    transmit(1, DAT_000174c2, strlen(DAT_000174c2)); // Extra newline at the end? (Original has it)
  }
}

// Function: process_input
// Processes a single character input and updates the Game Boy's joypad state.
// Returns false if 'q' (quit) was pressed, true otherwise.
bool process_input(struct GBState* gb_context) {
  char input_char;
  int bytes_read;
  
  // Attempt to receive 1 byte from stdin (fd 0)
  if (receive(0, &input_char, 1, &bytes_read) == 0 && bytes_read == 1) {
    switch(input_char) {
    case '?':
    case 'h':
      printf("HELP: (q)uit, (h)elp, (wasd) direction keys, (j) A, (k) B, (u) SELECT, (i) START\n");
      break;
    // Uppercase for key release (original logic)
    case 'A': gb_context->joypad_state &= ~2; break; // W
    case 'D': gb_context->joypad_state &= ~1; break; // A
    case 'I': gb_context->joypad_state &= ~0x80; break; // START
    case 'J': gb_context->joypad_state &= ~0x10; break; // A
    case 'K': gb_context->joypad_state &= ~0x20; break; // B
    case 'S': gb_context->joypad_state &= ~8; break; // D
    case 'U': gb_context->joypad_state &= ~0x40; break; // SELECT
    case 'W': gb_context->joypad_state &= ~4; break; // S
    // Lowercase for key press (original logic)
    case 'a': gb_context->joypad_state |= 2; break; // W
    case 'd': gb_context->joypad_state |= 1; break; // A
    case 'i': gb_context->joypad_state |= 0x80; break; // START
    case 'j': gb_context->joypad_state |= 0x10; break; // A
    case 'k': gb_context->joypad_state |= 0x20; break; // B
    case 'q':
      printf("Good-Bye\n");
      return false; // Quit signal
    case 's': gb_context->joypad_state |= 8; break; // D
    case 'u': gb_context->joypad_state |= 0x40; break; // SELECT
    case 'w': gb_context->joypad_state |= 4; break; // S
    }
  }
  return true; // Continue running
}

// Function: check_input
// Checks for available input on stdin without blocking.
// Returns false if a 'q' (quit) input was processed, true otherwise.
bool check_input(struct GBState* gb_context) {
  fd_set read_fds;
  struct timeval timeout = {0, 100}; // 0 seconds, 100 microseconds timeout
  int num_ready_fds = 0;
  
  FD_ZERO(&read_fds);
  FD_SET(0, &read_fds); // Check stdin (file descriptor 0)

  // fdwait is a wrapper around select.
  // The original parameters were: 1, local_90, 0, &local_9c, &local_94
  // which corresponds to: nfds=1 (for fd 0), read_fds, write_fds=NULL, timeout, num_ready_ptr
  int select_result = fdwait(1, &read_fds, NULL, &timeout, &num_ready_fds);

  if (select_result == 0) { // fdwait returned successfully
    if (num_ready_fds > 0) { // Input is available on stdin
      return process_input(gb_context);
    }
    // No input ready, but no error. Continue.
    return true;
  }
  // Error occurred in fdwait
  return false;
}

// Function: main
int main(int argc, char *argv[]) {
  if (argc != 2) {
    usage(argv[0]);
    exit(1);
  }

  struct GBState* gb_context = (struct GBState*)gb_new();
  if (gb_context == NULL) {
    fprintf(stderr, "Unable to allocate memory for Game Boy context.\n");
    exit(1);
  }

  printf("\x1b[2J"); // Clear screen
  
  if (!load_rom(gb_context, argv[1])) {
    fprintf(stderr, "Unable to load ROM from file: %s\n", argv[1]);
    exit(2);
  }

  printf("\x1b[?25l"); // Hide cursor
  gb_reset(gb_context);

  unsigned int frame_counter = 0;
  unsigned int input_check_timer = 0; // For timing input checks

  while (gb_tick(gb_context)) { // gb_tick returns true to continue, false to stop
    // Check if a frame has been rendered (indicated by int_val_0x44)
    if (gb_context->int_val_0x44 != 0) {
      frame_counter++;
      // Draw screen every 30 frames (0x1e)
      // The original check `local_18 == (local_18 / 0x1e) * 0x1e` is equivalent to `local_18 % 0x1e == 0`
      if (frame_counter % 30 == 0) {
        draw_screen(gb_context);
      }
      gb_context->int_val_0x44 = 0; // Reset frame rendered flag
    }

    input_check_timer++;
    // Check input when float_val_0x40 matches input_check_timer
    // This seems to be a custom timing mechanism.
    if (gb_context->float_val_0x40 == (float)input_check_timer) {
      if (!check_input(gb_context)) { // If check_input returns false (quit signal)
        break;
      }
      input_check_timer = 0; // Reset timer after input check
    }
  }

  print_reg(gb_context); // Print final register state
  printf("\x1b[?25h"); // Show cursor again (cleanup)
  printf("\n"); // Ensure prompt is on a new line
  // Assuming there's a gb_free(gb_context) counterpart, but not provided.
  return 0;
}