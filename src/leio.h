#pragma once

#include <stdio.h>

int fread_le(void *dest, size_t size, FILE *file);
int fwrite_le(void *dest, size_t size, FILE *file);
