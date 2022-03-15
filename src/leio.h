#pragma once

#include <stdio.h>

size_t fread_le(void *dest, size_t size, FILE *file);
size_t fwrite_le(void *dest, size_t size, FILE *file);
