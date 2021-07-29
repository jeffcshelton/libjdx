#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t major, minor, patch;
} JDXVersion;

const JDXVersion JDX_VERSION = { 0, 1, 0 };

typedef enum {
    JDXColorType_RGB = 3,
    JDXColorType_RGBA = 4,
    JDXColorType_GRAY = 1
} JDXColorType;

typedef struct {
    uint8_t *pixels;

    int16_t width, height;
    JDXColorType colorType;
} JDXImage;

typedef int16_t JDXLabel;

typedef struct {
    JDXVersion version;
    JDXColorType colorType;

    int16_t imageWidth, imageHeight;
    int64_t imageCount;
    int64_t bodySize;

    const char *error;
} JDXHeader;

typedef struct {
    JDXVersion version;

    JDXImage *images;
    JDXLabel *labels;
    int64_t itemCount;

    const char *error;
} JDXObject;
