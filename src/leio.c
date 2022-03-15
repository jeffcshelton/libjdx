#include "leio.h"

#include <stdbool.h>
#include <stdint.h>

static inline bool machine_is_le(void) {
  static int_fast8_t precheck = -1;

  if (precheck == -1) {
    uint64_t mem = 0x000000FF;
    precheck = ((unsigned char *) &mem)[0] == 0xFF;
  }

  return (bool) precheck;
}

size_t fread_le(void *dest, size_t size, FILE *file) {
  if (machine_is_le()) {
    return fread(dest, size, 1, file) == 1 ? size : EOF;
  } else {
    for (size_t i = size - 1; i >= 0; i--) {
      if ((((char *) dest)[i] = getc(file)) == EOF) {
        return EOF;
      }
    }

    return 0;
  }
}

size_t fwrite_le(void *src, size_t size, FILE *file) {
  if (machine_is_le()) {
    return fwrite(src, size, 1, file) == 1 ? size : EOF;
  } else {
    for (size_t i = size - 1; i >= 0; i--) {
      if (putc(((char *) src)[i], file) == EOF) {
        return EOF;
      }
    }
  }

  return 0;
}
