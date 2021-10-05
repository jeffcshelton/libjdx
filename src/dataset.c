#include "libjdx.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libdeflate.h>

static inline JDXDataset construct_error(const char *error_msg) {
    JDXDataset error_struct;
    memset(&error_struct, 0, sizeof(JDXDataset));

    error_struct.error = error_msg;
    return error_struct;
}

JDXDataset JDX_ReadDatasetFromFile(FILE *file) {
    JDXHeader header = JDX_ReadHeaderFromFile(file);

    if (header.error)
        return construct_error(header.error);

    JDXDataset dataset = {
        header, // header
        malloc(header.item_count * sizeof(JDXImage)), // images
        malloc(header.item_count * sizeof(JDXLabel)), // labels
        NULL // error
    };

    uint8_t *compressed_body = malloc(header.compressed_size);
    fread(compressed_body, 1, header.compressed_size, file);

    size_t image_size = (
        (size_t) header.image_width *
        (size_t) header.image_height *
        (size_t) header.color_type
    );

    size_t decompressed_body_size = (image_size + sizeof(JDXLabel)) * (size_t) header.item_count;
    uint8_t *decompressed_body = malloc(decompressed_body_size);

    decompressed_body = malloc(decompressed_body_size);

    // Decode/decompress encoded body and free the decompressor
    struct libdeflate_decompressor *decompressor = libdeflate_alloc_decompressor();
    enum libdeflate_result decompress_result = libdeflate_deflate_decompress(
        decompressor, compressed_body, header.compressed_size,
        decompressed_body, decompressed_body_size, NULL
    );

    libdeflate_free_decompressor(decompressor);

    if (decompress_result != LIBDEFLATE_SUCCESS)
        return construct_error("failed to decode");

    uint8_t *chunk_ptr = decompressed_body;
    for (int i = 0; i < header.item_count; i++) {
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
    // Write header to the file before the compressed image data
    JDX_WriteHeaderToFile(dataset.header, file);

    size_t image_size = (
        (size_t) dataset.header.image_width *
        (size_t) dataset.header.image_height *
        (size_t) dataset.header.color_type
    );

    size_t uncompressed_size = (size_t) (image_size + sizeof(JDXLabel)) * (size_t) dataset.header.item_count;

    uint8_t *uncompressed_body = malloc(uncompressed_size);
    uint8_t *body_ptr = uncompressed_body;

    for (int i = 0; i < dataset.header.item_count; i++) {
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

    fwrite(&compressed_size, sizeof(compressed_size), 1, file);
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
    for (int i = 0; i < dataset.header.item_count; i++)
        free(dataset.images[i].data);

    free(dataset.images);
    free(dataset.labels);
}
