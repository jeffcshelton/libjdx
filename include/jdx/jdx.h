#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t major, minor, patch;
} JDXVersion;

const JDXVersion JDX_VERSION = { 0, 1, 0 };

typedef enum {
    RGB = 3,
    RGBA = 4,
    GRAY = 1
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
} JDXHeader;

typedef struct {
    JDXVersion version;

    JDXImage *images;
    JDXLabel *labels;
    int64_t itemCount;
} JDXObject;

const JDXObject JDXOBJECT_ERROR = {
    { -1, -1, -1 },
    NULL,
    NULL,
    -1
};
