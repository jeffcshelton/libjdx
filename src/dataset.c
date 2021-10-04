#include "jdx/jdx.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libdeflate.h>

static inline JDXDataset construct_error(const char *error_msg) {
    JDXDataset error_struct = {
        { -1, -1, -1 },
        NULL, NULL, -1,
        error_msg
    };

    return error_struct;
}

JDXDataset JDX_ReadDatasetFromFile(FILE *file) {
    JDXHeader header = JDX_ReadHeaderFromFile(file);

    if (header.error)
        return construct_error(header.error);

    JDXDataset dataset = {
        header.version,
        malloc(header.image_count * sizeof(JDXImage)),
        malloc(header.image_count * sizeof(JDXLabel)),
        header.image_count,
        NULL
    };

    uint8_t *compressed_body = malloc(header.body_size);
    fread(compressed_body, 1, header.body_size, file);

    size_t image_size = (size_t) header.image_width *
                        (size_t) header.image_height *
                        (size_t) header.color_type;

    size_t decompressed_body_size = (image_size + 2) * (size_t) header.image_count;
    uint8_t *decompressed_body = malloc(decompressed_body_size);

    decompressed_body = malloc(decompressed_body_size);

    // Decode/decompress encoded body and free the decompressor
    struct libdeflate_decompressor *decompressor = libdeflate_alloc_decompressor();
    enum libdeflate_result decompress_result = libdeflate_deflate_decompress(
        decompressor, compressed_body, header.body_size,
        decompressed_body, decompressed_body_size, NULL
    );

    libdeflate_free_decompressor(decompressor);

    if (decompress_result != LIBDEFLATE_SUCCESS)
        return construct_error("failed to decode");

    uint8_t *chunk_ptr = decompressed_body;
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

        dataset.images[i] = img;
        dataset.labels[i] = label;
    }

    return dataset;
}

JDXDataset JDX_ReadDatasetFromPath(const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL)
        return construct_error("Cannot open file.");

    JDXDataset dataset = JDX_ReadDatasetFromFile(file);

    fclose(file);
    return dataset;
}

void JDX_WriteDatasetToFile(JDXDataset dataset, FILE *file) {
    // TODO: Reconsider implentation
    JDXColorType color_type = dataset.images[0].color_type;
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
    fwrite(&dataset.version.major, 1, 1, file);
    fwrite(&dataset.version.minor, 1, 1, file);
    fwrite(&dataset.version.patch, 1, 1, file);

    // Write the color signature, image width and height, and image count
    fwrite(color_signature, 1, 4, file);
    fwrite(&dataset.images[0].width, sizeof(int16_t), 1, file);
    fwrite(&dataset.images[0].height, sizeof(int16_t), 1, file);
    fwrite(&dataset.item_count, sizeof(int64_t), 1, file);

    size_t image_size = dataset.images[0].width * dataset.images[0].height * dataset.images[0].color_type;
    size_t uncompressed_size = dataset.item_count * (image_size + 2);

    uint8_t *uncompressed_body = malloc(uncompressed_size);
    uint8_t *body_ptr = uncompressed_body;

    for (int i = 0; i < dataset.item_count; i++) {
        memcpy(body_ptr, dataset.images[i].data, image_size);
        body_ptr += image_size;

        *((JDXLabel *) body_ptr) = dataset.labels[i];
        body_ptr += sizeof(JDXLabel);
    }

    // Must allocate for entire uncompressed size despite it almost certainly being less
    uint8_t *compressed_body = malloc(uncompressed_size);
    int64_t compressed_size;

    struct libdeflate_compressor *compressor = libdeflate_alloc_compressor(12);
    compressed_size = (int64_t) libdeflate_deflate_compress(
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
    fflush(file);

    free(uncompressed_body);
    free(compressed_body);
}

void JDX_WriteDatasetToPath(JDXDataset dataset, const char *path) {
    FILE *file = fopen(path, "wb");

    if (file == NULL)
        return;

    JDX_WriteDatasetToFile(dataset, file);
    fclose(file);
}

void JDX_FreeDataset(JDXDataset dataset) {
    for (int i = 0; i < dataset.item_count; i++)
        free(dataset.images[i].data);

    free(dataset.images);
    free(dataset.labels);
}
