#include <stdint.h> // For uint8_t, uint16_t, uint32_t
#include <unistd.h> // For read, write, ssize_t
#include <stdlib.h> // For exit, EXIT_FAILURE

// Global file descriptors
static int g_currentReadFD = -1;
static int g_currentWriteFD = -1;

// Function: setup_io
void setup_io(int read_fd, int write_fd) {
  g_currentReadFD = read_fd;
  g_currentWriteFD = write_fd;
}

// Function: read_u8
uint8_t read_u8(void) {
  uint8_t val;
  if (read(g_currentReadFD, &val, 1) != 1) {
    exit(EXIT_FAILURE);
  }
  return val;
}

// Function: read_u16
uint16_t read_u16(void) {
  uint16_t val;
  if (read(g_currentReadFD, &val, 2) != 2) {
    exit(EXIT_FAILURE);
  }
  return val;
}

// Function: read_u32
uint32_t read_u32(void) {
  uint32_t val;
  if (read(g_currentReadFD, &val, 4) != 4) {
    exit(EXIT_FAILURE);
  }
  return val;
}

// Function: write_u8
void write_u8(uint8_t val) {
  if (write(g_currentWriteFD, &val, 1) != 1) {
    exit(EXIT_FAILURE);
  }
}

// Function: write_u16
void write_u16(uint16_t val) {
  if (write(g_currentWriteFD, &val, 2) != 2) {
    exit(EXIT_FAILURE);
  }
}

// Function: write_u32
// Original function was missing a parameter and attempting to write from an invalid stack location.
// It is assumed to write a 32-bit unsigned integer value passed as an argument.
void write_u32(uint32_t val) {
  if (write(g_currentWriteFD, &val, 4) != 4) {
    exit(EXIT_FAILURE);
  }
}