#include "jdx/jdx.h"

#include <libdeflate.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static inline JDXHeader construct_error_header(const char *error) {
    JDXHeader error_header = {
        { -1, -1, -1 },
        -1, -1, -1, -1, -1,
        error
    };

    return error_header;
}

static inline JDXObject construct_error_object(const char *error) {
    JDXObject error_obj = {
        { -1, -1, -1 },
        NULL, NULL, -1,
        error
    };

    return error_obj;
}

JDXHeader JDX_ReadHeaderFromFile(FILE *file) {
    char corruption_check[3];
    fread(corruption_check, sizeof(corruption_check), 1, file);

    if (memcmp(corruption_check, "JDX", 3) != 0)
        return construct_error_header("corruption check failed");

    JDXHeader header;
    char color_signature[4];

    fread(&header.version, sizeof(JDXVersion), 1, file);
    fread(color_signature, sizeof(color_signature), 1, file);
    fread(&header.image_width, sizeof(header.image_width), 1, file);
    fread(&header.image_height, sizeof(header.image_height), 1, file);
    fread(&header.image_count, sizeof(header.image_count), 1, file);
    fread(&header.body_size, sizeof(header.body_size), 1, file);

    if (header.image_width < 0 || header.image_height < 0 || header.image_count < 0 || header.body_size < 0)
        return construct_error_header("invalid header item");

    JDXColorType color_type;
    if (memcmp(color_signature, "RGB8", 4) == 0) {
        color_type = JDXColorType_RGB;
    } else if (memcmp(color_signature, "RGBA", 4) == 0) {
        color_type = JDXColorType_RGBA;
    } else if (memcmp(color_signature, "GRAY", 4) == 0) {
        color_type = JDXColorType_GRAY;
    } else {
        return construct_error_header("invalid color signature");
    }

    header.color_type = color_type;
    header.error = NULL;
    return header;
}

JDXHeader JDX_ReadHeaderFromPath(const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL)
        return construct_error_header("cannot open file");

    JDXHeader header = JDX_ReadHeaderFromFile(file);

    fclose(file);
    return header;
}

JDXObject JDX_ReadObjectFromFile(FILE *file) {
    JDXHeader header = JDX_ReadHeaderFromFile(file);

    if (header.error)
        return construct_error_object(header.error);

    JDXObject obj = {
        header.version,
        malloc(header.image_count * sizeof(JDXImage)),
        malloc(header.image_count * sizeof(JDXLabel)),
        header.image_count,
        NULL
    };

    uint8_t *encoded_body = malloc(header.body_size);
    fread(encoded_body, 1, header.body_size, file);

    size_t image_size = (size_t) header.image_width *
                        (size_t) header.image_height *
                        (size_t) header.color_type;

    size_t decoded_body_size = (image_size + 2) * (size_t) header.image_count;
    uint8_t *decoded_body = malloc(decoded_body_size);

    // Decode/decompress encoded body and free the decompressor
    struct libdeflate_decompressor *decoder = libdeflate_alloc_decompressor();
    enum libdeflate_result decode_result = libdeflate_deflate_decompress(
        decoder, encoded_body, header.body_size,
        decoded_body, decoded_body_size, NULL
    );

    libdeflate_free_decompressor(decoder);

    if (decode_result != LIBDEFLATE_SUCCESS)
        return construct_error_object("failed to decode");

    uint8_t *chunk_ptr = decoded_body;
    for (int i = 0; i < header.image_count; i++) {
        // Set and allocate image
        JDXImage img = {
            malloc(image_size),
            header.image_width, header.image_height,
            header.color_type
        };

        // Copy image data into new image buffer and advance chunk ptr
        memcpy(img.pixels, chunk_ptr, image_size);
        chunk_ptr += image_size;

        // Type pun end of chunk into label and advance chunk ptr again
        JDXLabel label = *((int16_t *) chunk_ptr);
        chunk_ptr += sizeof(int16_t);

        obj.images[i] = img;
        obj.labels[i] = label;
    }

    return obj;
}

JDXObject JDX_ReadObjectFromPath(const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL)
        return construct_error_object("cannot open file");

    JDXObject obj = JDX_ReadObjectFromFile(file);

    fclose(file);
    return obj;
}
