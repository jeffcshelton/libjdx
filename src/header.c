#include "jdx/jdx.h"

#include <stdio.h>
#include <string.h>

const JDXVersion JDX_VERSION = { 0, 1, 0 };

static inline JDXHeader construct_error(const char *error_msg) {
    JDXHeader error_struct = {
        { -1, -1, -1 },
        -1, -1, -1, -1, -1,
        error_msg
    };

    return error_struct;
}

JDXHeader JDX_ReadHeaderFromFile(FILE *file) {
    char corruption_check[3];
    fread(corruption_check, sizeof(corruption_check), 1, file);

    if (memcmp(corruption_check, "JDX", 3) != 0)
        return construct_error("corruption check failed");

    JDXHeader header;
    char color_signature[4];

    fread(&header.version, sizeof(JDXVersion), 1, file);
    fread(color_signature, sizeof(color_signature), 1, file);
    fread(&header.image_width, sizeof(header.image_width), 1, file);
    fread(&header.image_height, sizeof(header.image_height), 1, file);
    fread(&header.image_count, sizeof(header.image_count), 1, file);
    fread(&header.body_size, sizeof(header.body_size), 1, file);

    if (header.image_width < 0 || header.image_height < 0 || header.image_count < 0 || header.body_size < 0)
        return construct_error("invalid header item");

    JDXColorType color_type;
    if (memcmp(color_signature, "RGB8", 4) == 0) {
        color_type = JDXColorType_RGB;
    } else if (memcmp(color_signature, "RGBA", 4) == 0) {
        color_type = JDXColorType_RGBA;
    } else if (memcmp(color_signature, "GRAY", 4) == 0) {
        color_type = JDXColorType_GRAY;
    } else {
        return construct_error("invalid color signature");
    }

    header.color_type = color_type;
    header.error = NULL;
    return header;
}

JDXHeader JDX_ReadHeaderFromPath(const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL)
        return construct_error("cannot open file");

    JDXHeader header = JDX_ReadHeaderFromFile(file);

    fclose(file);
    return header;
}
