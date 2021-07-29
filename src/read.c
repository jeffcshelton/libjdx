#include "jdx/jdx.h"

#include <libdeflate.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static const JDXHeader JDX_ERROR_HEADER = { { -1, -1, -1 }, -1, -1, -1, -1, -1, "unspecified error" };
static const JDXObject JDX_ERROR_OBJECT = { { -1, -1, -1 }, NULL, NULL, -1, "unspecified error" };

JDXHeader JDX_ReadHeaderFromFile(FILE *file) {
    char corruptionCheck[3];
    fread(corruptionCheck, sizeof(corruptionCheck), 1, file);

    if (memcmp(corruptionCheck, "JDX", 3) != 0)
        return JDX_ERROR_HEADER;

    JDXVersion version;
    char colorSignature[4];
    int16_t width, height;
    int64_t imageCount, bodySize;

    fread(&version, sizeof(JDXVersion), 1, file);
    fread(colorSignature, sizeof(colorSignature), 1, file);
    fread(&width, sizeof(width), 1, file);
    fread(&height, sizeof(height), 1, file);
    fread(&imageCount, sizeof(imageCount), 1, file);
    fread(&bodySize, sizeof(bodySize), 1, file);

    if (width < 0 || height < 0 || imageCount < 0 || bodySize < 0)
        return JDX_ERROR_HEADER;

    JDXColorType colorType;
    if (memcmp(colorSignature, "RGB8", 4) == 0) {
        colorType = JDXColorType_RGB;
    } else if (memcmp(colorSignature, "RGBA", 4) == 0) {
        colorType = JDXColorType_RGBA;
    } else if (memcmp(colorSignature, "GRAY", 4) == 0) {
        colorType = JDXColorType_GRAY;
    } else {
        return JDX_ERROR_HEADER;
    }

    JDXHeader header = {
        version,
        colorType,
        width, height,
        imageCount,
        bodySize
    };

    return header;
}

JDXHeader JDX_ReadHeaderFromPath(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) return JDX_ERROR_HEADER;

    JDXHeader header = JDX_ReadHeaderFromFile(file);
    fclose(file);

    return header;
}
