#include "jdx/jdx.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libdeflate.h>

static inline JDXObject construct_error_object(const char *error) {
    JDXObject error_obj = {
        { -1, -1, -1 },
        NULL, NULL, -1,
        error
    };

    return error_obj;
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
        memcpy(img.data, chunk_ptr, image_size);
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

void JDX_WriteObjectToFile(JDXObject obj, FILE *file) {
    // TODO: Reconsider implentation
    JDXColorType color_type = obj.images[0].color_type;
    const char *color_signature = NULL;
    
    if (color_type == JDXColorType_GRAY) {
        color_signature = "GRAY";
    } else if (color_type == JDXColorType_RGB) {
        color_signature = "RGB8";
    } else if (color_type == JDXColorType_RGBA) {
        color_signature = "RGBA";
    } else {
        errno = EINVAL;
        return;
    }

    // Write string "JDX" followed by version info
    fwrite("JDX", 1, 3, file);
    fwrite(&obj.version.major, 1, 1, file);
    fwrite(&obj.version.minor, 1, 1, file);
    fwrite(&obj.version.patch, 1, 1, file);

    // Write the color signature, image width and height, and image count
    fwrite(color_signature, 1, 4, file);
    fwrite(&obj.images[0].width, sizeof(int16_t), 1, file);
    fwrite(&obj.images[0].height, sizeof(int16_t), 1, file);
    fwrite(&obj.item_count, sizeof(int64_t), 1, file);

    size_t image_size = obj.images[0].width * obj.images[0].height * obj.images[0].color_type;
    size_t uncompressed_size = obj.item_count * (image_size + 2);

    uint8_t *uncompressed_body = malloc(uncompressed_size);
    uint8_t *body_ptr = uncompressed_body;

    for (int i = 0; i < obj.item_count; i++) {
        memcpy(body_ptr, obj.images[i].data, image_size);
        body_ptr += image_size;

        *((JDXLabel *) body_ptr) = obj.labels[i];
        body_ptr += sizeof(JDXLabel);
    }

    // Must allocate for entire uncompressed size despite it almost certainly being less
    uint8_t *compressed_body = malloc(uncompressed_size);

    struct libdeflate_compressor *compressor = libdeflate_alloc_compressor(12);
    int64_t compressed_size = libdeflate_deflate_compress(
        compressor,
        uncompressed_body,
        uncompressed_size,
        compressed_body,
        uncompressed_size
    );

    libdeflate_free_compressor(compressor);

    // libdeflate will return 0 if operation failed
    if (compressed_size == 0) {
        errno = EPROTO;
        return;
    }

    fwrite(&compressed_size, sizeof(int64_t), 1, file);
    fwrite(compressed_body, 1, compressed_size, file);
    fclose(file);

    free(uncompressed_body);
    free(compressed_body);
}

void JDX_FreeObject(JDXObject obj) {
    for (int i = 0; i < obj.item_count; i++)
        free(obj.images[i].data);

    free(obj.images);
    free(obj.labels);
}
