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

    if (memcmp(corruption_check, "JDX", 3) != 0) {
        JDXHeader error_header = JDX_ERROR_HEADER;
        error_header.error = "corruption check failed";

        return error_header;
    }

    JDXHeader header;
    char color_signature[4];

    fread(&header.version, sizeof(JDXVersion), 1, file);
    fread(color_signature, sizeof(color_signature), 1, file);
    fread(&header.image_width, sizeof(header.image_width), 1, file);
    fread(&header.image_height, sizeof(header.image_height), 1, file);
    fread(&header.image_count, sizeof(header.image_count), 1, file);
    fread(&header.body_size, sizeof(header.body_size), 1, file);

    if (header.image_width < 0 || header.image_height < 0 || header.image_count < 0 || header.body_size < 0) {
        JDXHeader error_header = JDX_ERROR_HEADER;
        error_header.error = "invalid negative value";

        return error_header;
    }

    JDXColorType color_type;
    if (memcmp(color_signature, "RGB8", 4) == 0) {
        color_type = JDXColorType_RGB;
    } else if (memcmp(color_signature, "RGBA", 4) == 0) {
        color_type = JDXColorType_RGBA;
    } else if (memcmp(color_signature, "GRAY", 4) == 0) {
        color_type = JDXColorType_GRAY;
    } else {
        JDXHeader error_header = JDX_ERROR_HEADER;
        error_header.error = "invalid color signature";

        return error_header;
    }

    header.color_type = color_type;
    header.error = NULL;
    return header;
}

JDXHeader JDX_ReadHeaderFromPath(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) return JDX_ERROR_HEADER;

    JDXHeader header = JDX_ReadHeaderFromFile(file);
    fclose(file);

    return header;
}
