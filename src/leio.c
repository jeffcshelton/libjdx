#include "leio.h"

#include <stdint.h>

static int cpu_is_le = -1;

static inline void check_le(void) {
  if (cpu_is_le == -1) {
    uint64_t t = 0x000000FF;
    cpu_is_le = ((unsigned char *) &t)[0] == 0xFF;
  }
}

int fread_le(void *dest, size_t size, FILE *file) {
  check_le();

  if (cpu_is_le) {
    return fread(dest, size, 1, file) == 1 ? 0 : EOF;
  } else {
    for (size_t i = size - 1; i >= 0; i--) {
      if ((((char *) dest)[i] = getc(file)) == EOF) {
        return EOF;
      }
    }

    return 0;
  }
}

int fwrite_le(void *src, size_t size, FILE *file) {
  check_le();

  if (cpu_is_le) {
    return fwrite(src, size, 1, file) == 1 ? 0 : EOF;
  } else {
    for (size_t i = size - 1; i >= 0; i--) {
      if (putc(((char *) src)[i], file) == EOF) {
        return EOF;
      }
    }
  }

  return 0;
}
