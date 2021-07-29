#include "jdx/jdx.h"

#include <libdeflate.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static const JDXHeader JDX_ERROR_HEADER = { { -1, -1, -1 }, -1, -1, -1, -1, -1, "unspecified error" };
static const JDXObject JDX_ERROR_OBJECT = { { -1, -1, -1 }, NULL, NULL, -1, "unspecified error" };

JDXHeader JDX_ReadHeaderFromFile(FILE *file) {
    char corruption_check[3];
    fread(corruption_check, sizeof(corruption_check), 1, file);

    if (memcmp(corruption_check, "JDX", 3) != 0)
        return JDX_ERROR_HEADER;

    JDXVersion version;
    char color_signature[4];
    int16_t width, height;
    int64_t image_count, body_size;

    fread(&version, sizeof(JDXVersion), 1, file);
    fread(color_signature, sizeof(color_signature), 1, file);
    fread(&width, sizeof(width), 1, file);
    fread(&height, sizeof(height), 1, file);
    fread(&image_count, sizeof(image_count), 1, file);
    fread(&body_size, sizeof(body_size), 1, file);

    if (width < 0 || height < 0 || image_count < 0 || body_size < 0)
        return JDX_ERROR_HEADER;

    JDXColorType color_type;
    if (memcmp(color_signature, "RGB8", 4) == 0) {
        color_type = JDXColorType_RGB;
    } else if (memcmp(color_signature, "RGBA", 4) == 0) {
        color_type = JDXColorType_RGBA;
    } else if (memcmp(color_signature, "GRAY", 4) == 0) {
        color_type = JDXColorType_GRAY;
    } else {
        return JDX_ERROR_HEADER;
    }

    JDXHeader header = {
        version,
        color_type,
        width, height,
        image_count,
        body_size
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
