#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	JDXBuildType_DEV,
	JDXVBuildType_ALPHA,
	JDXBuildType_BETA,
	JDXBuildType_RC,
	JDXBuildType_RELEASE,
} JDXBuildType;

typedef struct {
	uint8_t major, minor, patch;
	JDXBuildType release_type;
} JDXVersion;

extern const JDXVersion JDX_VERSION;

typedef enum {
	JDXError_NONE, // must be zero by standard

	JDXError_OPEN_FILE,
	JDXError_CLOSE_FILE,
	JDXError_READ_FILE,
	JDXError_WRITE_FILE,
	JDXError_CORRUPT_FILE,

	JDXError_UNEQUAL_WIDTHS,
	JDXError_UNEQUAL_HEIGHTS,
	JDXError_UNEQUAL_BIT_DEPTHS
} JDXError;

typedef uint16_t JDXLabel;

typedef struct {
	uint8_t *data;

	uint16_t width, height;
	uint8_t bit_depth;

	JDXLabel label;
} JDXItem;

typedef struct {
	JDXVersion version;

	uint16_t image_width, image_height;
	uint8_t bit_depth;

	uint64_t item_count;
	uint64_t compressed_size;
} JDXHeader;

typedef struct {
	JDXHeader header;
	JDXItem *items;
} JDXDataset;

JDXError JDX_ReadHeaderFromFile(JDXHeader *dest, FILE *file);
JDXError JDX_ReadHeaderFromPath(JDXHeader *dest, const char *path);

JDXError JDX_ReadDatasetFromFile(JDXDataset *dest, FILE *file);
JDXError JDX_ReadDatasetFromPath(JDXDataset *dest, const char *path);

JDXError JDX_WriteHeaderToFile(JDXHeader header, FILE *file);

JDXError JDX_WriteDatasetToFile(JDXDataset dataset, FILE *file);
JDXError JDX_WriteDatasetToPath(JDXDataset dataset, const char *path);

void JDX_CopyDataset(JDXDataset src, JDXDataset *dest);
JDXError JDX_AppendDataset(JDXDataset *dest, JDXDataset src);

void JDX_FreeDataset(JDXDataset dataset);

#ifdef __cplusplus
}
#endif
