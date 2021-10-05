#include "libjdx.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

const JDXVersion JDX_VERSION = { 0, 1, 0 };

static inline JDXHeader construct_error(const char *error_msg) {
    JDXHeader error_struct;
    memset(&error_struct, 0, sizeof(error_struct));

    error_struct.error = error_msg;
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
    fread(&header.item_count, sizeof(header.item_count), 1, file);
    fread(&header.compressed_size, sizeof(header.compressed_size), 1, file);

    if (header.image_width < 0 || header.image_height < 0 || header.item_count < 0 || header.compressed_size < 0)
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

void JDX_WriteHeaderToFile(JDXHeader header, FILE *file) {
    const char *color_signature = NULL;

    if (header.color_type == JDXColorType_GRAY) {
        color_signature = "GRAY";
    } else if (header.color_type == JDXColorType_RGB) {
        color_signature = "RGB8";
    } else if (header.color_type == JDXColorType_RGBA) {
        color_signature = "RGBA";
    } else {
        errno = EINVAL;
        return;
    }

    // Write string "JDX" to file followed by version info
    fwrite("JDX", 1, 3, file);
    fwrite(&header.version.major, 1, 1, file);
    fwrite(&header.version.minor, 1, 1, file);
    fwrite(&header.version.patch, 1, 1, file);

    // Write the color signature, image width and height, and image count
    fwrite(color_signature, 1, 4, file);
    fwrite(&header.image_width, sizeof(header.image_width), 1, file);
    fwrite(&header.image_height, sizeof(header.image_height), 1, file);
    fwrite(&header.item_count, sizeof(header.item_count), 1, file);

    fflush(file);
}
