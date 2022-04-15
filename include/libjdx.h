#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JDX_MAX_LABEL_LEN 128

#define JDX_BUILD_DEV 0
#define JDX_BUILD_ALPHA 1
#define JDX_BUILD_BETA 2
#define JDX_BUILD_RC 3
#define JDX_BUILD_RELEASE 4

typedef enum {
	JDXError_NONE, // must be zero by standard

	JDXError_OPEN_FILE,
	JDXError_CLOSE_FILE,
	JDXError_READ_FILE,
	JDXError_WRITE_FILE,
	JDXError_CORRUPT_FILE,

	JDXError_MEMORY_FAILURE,

	JDXError_UNEQUAL_WIDTHS,
	JDXError_UNEQUAL_HEIGHTS,
	JDXError_UNEQUAL_BIT_DEPTHS
} JDXError;

typedef struct {
	uint8_t build_type, patch, minor, major;
} JDXVersion;

typedef struct {
	JDXVersion version;

	uint64_t image_count;
	uint16_t image_width, image_height;
	uint8_t bit_depth;

	const char **labels;
	uint16_t label_count;
} JDXHeader;

typedef struct {
	JDXHeader *header;

	uint16_t *_raw_labels;
	uint8_t *_raw_image_data;
} JDXDataset;

typedef struct {
	uint8_t *raw_data;

	uint16_t image_width, image_height;
	uint8_t bit_depth;

	char *label;
	uint16_t label_index;
} JDXImage;

static const JDXVersion JDX_VERSION = { JDX_BUILD_ALPHA, 0, 4, 0 };

int32_t JDX_CompareVersions(JDXVersion v1, JDXVersion v2);

JDXHeader *JDX_AllocHeader(void);
void JDX_FreeHeader(JDXHeader *header);

void JDX_CopyHeader(JDXHeader *dest, const JDXHeader *src);

size_t JDX_GetImageSize(JDXHeader *header);

JDXError JDX_ReadHeaderFromFile(JDXHeader *dest, FILE *file);
JDXError JDX_ReadHeaderFromPath(JDXHeader *dest, const char *path);
JDXError JDX_WriteHeaderToFile(JDXHeader *header, FILE *file);

JDXDataset *JDX_AllocDataset(void);
void JDX_FreeDataset(JDXDataset *dataset);

void JDX_CopyDataset(JDXDataset *dest, const JDXDataset *src);
JDXError JDX_AppendDataset(JDXDataset *dest, const JDXDataset *src);

JDXImage *JDX_GetImage(JDXDataset *dataset, uint64_t index);

JDXError JDX_ReadDatasetFromFile(JDXDataset *dest, FILE *file);
JDXError JDX_ReadDatasetFromPath(JDXDataset *dest, const char *path);
JDXError JDX_WriteDatasetToFile(JDXDataset *dataset, FILE *file);
JDXError JDX_WriteDatasetToPath(JDXDataset *dataset, const char *path);

void JDX_FreeImage(JDXImage *image);

#ifdef __cplusplus
}
#endif
