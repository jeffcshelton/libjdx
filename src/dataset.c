#include "libjdx.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libdeflate.h>

// TODO: For creating, copying, and appending JDXDataset consider using block memory allocation instead of many mallocs

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

JDXDataset JDX_CopyDataset(JDXDataset dataset) {
    JDXDataset copy = {
        dataset.header, // header
        malloc(dataset.header.item_count * sizeof(JDXImage)), // images
        malloc(dataset.header.item_count * sizeof(JDXLabel)), // labels
        dataset.error // error
    };

    size_t image_size = (
        (size_t) dataset.header.image_width *
        (size_t) dataset.header.image_height *
        (size_t) dataset.header.color_type
    );

    for (int i = 0; i < dataset.header.item_count; i++) {
        JDXImage image_copy = {
            malloc(image_size),
            dataset.header.image_width,
            dataset.header.image_height,
            dataset.header.color_type
        };

        memcpy(image_copy.data, dataset.images[i].data, image_size);
        copy.images[i] = image_copy;
    }

    return copy;
}

void JDX_AppendDataset(JDXDataset *dest, JDXDataset src) {
    // Check for any compatibility errors between the two datasets
    if (src.header.image_width != dest->header.image_width) {
        dest->error = "mismatching image widths";
    } else if (src.header.image_height != dest->header.image_height) {
        dest->error = "mismatching image heights";
    } else if (src.header.color_type != dest->header.color_type) {
        dest->error = "mismatching color types";
    }

    // Calculate final item count and realloc destination arrays accordingly
    int64_t new_item_count = dest->header.item_count + src.header.item_count;
    dest->images = realloc(dest->images, new_item_count * sizeof(JDXImage));
    dest->labels = realloc(dest->labels, new_item_count * sizeof(JDXLabel));

    size_t image_size = (
        (size_t) src.header.image_width *
        (size_t) src.header.image_height *
        (size_t) src.header.color_type
    );

    // Copy each image and label individually and store them in dest
    for (int s = 0, d = dest->header.item_count; s < src.header.item_count; s++, d++) {
        JDXImage copy_image = {
            malloc(image_size), // data
            src.header.image_width, // width
            src.header.image_height, // height
            src.header.color_type // color_type
        };

        memcpy(copy_image.data, src.images[s].data, image_size);

        dest->images[d] = copy_image;
        dest->labels[d] = src.labels[s];
    }

    // Set destination item count
    dest->header.item_count = new_item_count;
}

void JDX_FreeDataset(JDXDataset dataset) {
    for (int i = 0; i < dataset.header.item_count; i++)
        free(dataset.images[i].data);

    free(dataset.images);
    free(dataset.labels);
}
