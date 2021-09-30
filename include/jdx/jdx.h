#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t major, minor, patch;
} JDXVersion;

extern const JDXVersion JDX_VERSION;

typedef enum {
    JDXColorType_RGB = 3,
    JDXColorType_RGBA = 4,
    JDXColorType_GRAY = 1
} JDXColorType;

typedef struct {
    uint8_t *data;

    int16_t width, height;
    JDXColorType color_type;
} JDXImage;

typedef int16_t JDXLabel;

typedef struct {
    JDXVersion version;
    JDXColorType color_type;

    int16_t image_width, image_height;
    int64_t image_count;
    int64_t body_size;

    const char *error;
} JDXHeader;

typedef struct {
    JDXVersion version;

    JDXImage *images;
    JDXLabel *labels;
    int64_t item_count;

    const char *error;
} JDXDataset;

JDXHeader JDX_ReadHeaderFromFile(FILE *file);
JDXHeader JDX_ReadHeaderFromPath(const char *path);

JDXDataset JDX_ReadDatasetFromFile(FILE *file, void (*external_deflate_decompress)(const uint8_t *compressed, uint8_t *decompressed, size_t compressed_size, size_t decompressed_size));
JDXDataset JDX_ReadDatasetFromPath(const char *path, void (*external_deflate_decompress)(const uint8_t *compressed, uint8_t *decompressed, size_t compressed_size, size_t decompressed_size));

void JDX_WriteDatasetToFile(JDXDataset obj, FILE *file, size_t (*external_deflate_compress)(const uint8_t *uncompressed, uint8_t *compressed, size_t uncompressed_size));
void JDX_WriteDatasetToPath(JDXDataset obj, const char *path, size_t (*external_deflate_compress)(const uint8_t *uncompressed, uint8_t *compressed, size_t uncompressed_size));

void JDX_FreeDataset(JDXDataset obj);

#ifdef __cplusplus
}
#endif
