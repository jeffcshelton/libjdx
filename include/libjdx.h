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

typedef union {
	struct {
		uint8_t build_type, patch, minor, major;
	};

	int32_t raw;
} JDXVersion;

extern const JDXVersion JDX_VERSION;

int32_t JDX_CompareVersions(JDXVersion v1, JDXVersion v2);

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

	const char **labels;
	uint16_t label_count;

	uint64_t item_count;
	uint64_t compressed_size;
} JDXHeader;

typedef struct {
	JDXHeader *header;
	JDXItem *items;
} JDXDataset;

JDXHeader *JDX_AllocHeader(void);
void JDX_FreeHeader(JDXHeader *header);

void JDX_CopyHeader(JDXHeader *dest, JDXHeader *src);

JDXError JDX_ReadHeaderFromFile(JDXHeader *dest, FILE *file);
JDXError JDX_ReadHeaderFromPath(JDXHeader *dest, const char *path);
JDXError JDX_WriteHeaderToFile(JDXHeader *header, FILE *file);

JDXDataset *JDX_AllocDataset(void);
void JDX_FreeDataset(JDXDataset *dataset);

void JDX_CopyDataset(JDXDataset *dest, JDXDataset *src);
JDXError JDX_AppendDataset(JDXDataset *dest, JDXDataset *src);

JDXError JDX_ReadDatasetFromFile(JDXDataset *dest, FILE *file);
JDXError JDX_ReadDatasetFromPath(JDXDataset *dest, const char *path);
JDXError JDX_WriteDatasetToFile(JDXDataset *dataset, FILE *file);
JDXError JDX_WriteDatasetToPath(JDXDataset *dataset, const char *path);

#ifdef __cplusplus
}
#endif
