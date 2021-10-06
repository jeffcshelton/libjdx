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
    int64_t item_count;
    int64_t compressed_size;

    const char *error;
} JDXHeader;

typedef struct {
    JDXHeader header;

    JDXImage *images;
    JDXLabel *labels;

    const char *error;
} JDXDataset;

JDXHeader JDX_ReadHeaderFromFile(FILE *file);
JDXHeader JDX_ReadHeaderFromPath(const char *path);

JDXDataset JDX_ReadDatasetFromFile(FILE *file);
JDXDataset JDX_ReadDatasetFromPath(const char *path);

void JDX_WriteHeaderToFile(JDXHeader header, FILE *file);

void JDX_WriteDatasetToFile(JDXDataset dataset, FILE *file);
void JDX_WriteDatasetToPath(JDXDataset dataset, const char *path);

JDXDataset JDX_CopyDataset(JDXDataset dataset);
void JDX_AppendDataset(JDXDataset *dest, JDXDataset src);

void JDX_FreeDataset(JDXDataset dataset);

#ifdef __cplusplus
}
#endif
