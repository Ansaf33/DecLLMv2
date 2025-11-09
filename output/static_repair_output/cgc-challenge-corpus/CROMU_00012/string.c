#include <stddef.h> // For size_t

// Function: memcpy
void *memcpy(void *__dest, void *__src, size_t __n) {
  unsigned char *d = (unsigned char *)__dest;
  unsigned char *s = (unsigned char *)__src;

  while (__n--) {
    *d++ = *s++;
  }

  return __dest;
}