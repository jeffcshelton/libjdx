#include "libjdx.h"
#include "leio.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define SIGN(x) ((x) > 0 ? 1 : -1)

const JDXVersion JDX_VERSION = { 0, 4, 0, JDXBuildType_DEV };

JDXHeader *JDX_AllocHeader(void) {
	return calloc(1, sizeof(JDXHeader));
}

void JDX_FreeHeader(JDXHeader *header) {
	if (header == NULL) {
		return;
	}

	if (header->labels) {
		if (header->label_count > 0) {
			for (uint_least16_t l = 0; l < header->label_count; l++) {
				free((char *) header->labels[l]);
			}
		}

		free(header->labels);
	}
}

void JDX_CopyHeader(JDXHeader *src, JDXHeader *dest) {
	JDX_FreeHeader(dest);

	dest->version = src->version;
	dest->image_width = src->image_width;
	dest->image_height = src->image_height;
	dest->bit_depth = src->bit_depth;

	dest->labels = malloc(src->label_count * sizeof(char **));

	for (uint_least16_t l = 0; l < src->label_count; l++) {
		size_t label_size = strlen(src->labels[l]) + 1;

		dest->labels[l] = malloc(label_size);
		memcpy((char *) dest->labels[l], src->labels[l], label_size);
	}

	dest->label_count = src->label_count;

	dest->item_count = src->item_count;
	dest->compressed_size = src->compressed_size;
}

int32_t JDX_CompareVersions(JDXVersion v1, JDXVersion v2) {
	if (v1.major != v2.major) {
		return SIGN((int32_t) v1.major - (int32_t) v2.major);
	} else if (v1.minor != v2.minor) {
		return SIGN((int32_t) v1.minor - (int32_t) v2.minor);
	} else if (v1.patch != v2.patch) {
		return SIGN((int32_t) v1.patch - (int32_t) v2.patch);
	} else if (v1.build_type != v2.build_type) {
		return (int32_t) SIGN(v1.build_type - v2.build_type);
	}

	return 0;
}

JDXError JDX_ReadHeaderFromFile(JDXHeader *dest, FILE *file) {
	char corruption_check[3];

	if (fread(corruption_check, 1, sizeof(corruption_check), file) != sizeof(corruption_check))
		return JDXError_READ_FILE;

	if (memcmp(corruption_check, "JDX", 3) != 0)
		return JDXError_CORRUPT_FILE;

	JDXHeader header;
	uint8_t build_type;

	if (
		fread_le(&header.version.major, sizeof(header.version.major), file) == EOF ||
		fread_le(&header.version.minor, sizeof(header.version.minor), file) == EOF ||
		fread_le(&header.version.patch, sizeof(header.version.patch), file) == EOF ||
		fread_le(&build_type, sizeof(build_type), file) == EOF ||
		fread_le(&header.image_width, sizeof(header.image_width), file) == EOF ||
		fread_le(&header.image_height, sizeof(header.image_height), file) == EOF ||
		fread_le(&header.bit_depth, sizeof(header.bit_depth), file) == EOF ||
		fread_le(&header.label_count, sizeof(header.label_count), file) == EOF
	) return JDXError_READ_FILE;

	char **labels = malloc(header.label_count * sizeof(char *));

	char buf[MAX_LABEL_LEN];
	int i;

	for (int_fast16_t l = 0; l < header.label_count; l++) {
		i = 0;
		while (i < MAX_LABEL_LEN && (buf[i++] = getc(file)));

		if (buf[i - 1]) {
			while (l >= 0) {
				free(labels[--l]);
			}

			free(labels);
			return JDXError_CORRUPT_FILE;
		}

		char *label = malloc(i);
		memcpy(label, buf, i);

		labels[l] = label;
	}

	header.labels = (const char **) labels;

	if (
		fread_le(&header.item_count, sizeof(header.item_count), file) == EOF ||
		fread_le(&header.compressed_size, sizeof(header.compressed_size), file) == EOF
	) {
		JDX_FreeHeader(&header);
		return JDXError_READ_FILE;
	}

	header.version.build_type = (JDXBuildType) build_type;

	if ((header.bit_depth != 8 && header.bit_depth != 24 && header.bit_depth != 32) || (header.version.build_type > JDXBuildType_RELEASE)) {
		JDX_FreeHeader(&header);
		return JDXError_CORRUPT_FILE;
	}

	*dest = header;
	return JDXError_NONE;
}

JDXError JDX_ReadHeaderFromPath(JDXHeader *dest, const char *path) {
	FILE *file = fopen(path, "rb");

	if (file == NULL)
		return JDXError_OPEN_FILE;

	JDXError error = JDX_ReadHeaderFromFile(dest, file); // Named 'error' but could (and should) be 'JDXError_NONE'

	if (fclose(file) == EOF)
		return JDXError_CLOSE_FILE;

	return error;
}

JDXError JDX_WriteHeaderToFile(JDXHeader header, FILE *file) {
	char corruption_check[3] = {'J', 'D', 'X'};

	if (fwrite(corruption_check, 1, sizeof(corruption_check), file) != sizeof(corruption_check))
		return JDXError_WRITE_FILE;

	uint8_t build_type = (uint8_t) header.version.build_type;

	// Must write this way to account for alignment of JDXHeader
	if (
		fwrite_le(&header.version.major, sizeof(header.version.major), file) == EOF ||
		fwrite_le(&header.version.minor, sizeof(header.version.minor), file) == EOF ||
		fwrite_le(&header.version.patch, sizeof(header.version.patch), file) == EOF ||
		fwrite_le(&build_type, sizeof(build_type), file) == EOF ||
		fwrite_le(&header.image_width, sizeof(header.image_width), file) == EOF ||
		fwrite_le(&header.image_height, sizeof(header.image_height), file) == EOF ||
		fwrite_le(&header.bit_depth, sizeof(header.bit_depth), file) == EOF ||
		fwrite_le(&header.label_count, sizeof(header.label_count), file) == EOF
	) return JDXError_WRITE_FILE;

	const char *label;
	int len;

	for (int_fast16_t l = 0; l < header.label_count; l++) {
		label = header.labels[l];
		len = strlen(label) + 1;

		if (fwrite(label, sizeof(char), len, file) != len) {
			return JDXError_WRITE_FILE;
		}
	}

	if (
		fwrite_le(&header.item_count, sizeof(header.item_count), file) == EOF ||
		fwrite_le(&header.compressed_size, sizeof(header.compressed_size), file) == EOF
	) return JDXError_WRITE_FILE;

	if (fflush(file) == EOF)
		return JDXError_WRITE_FILE;

	return JDXError_NONE;
}
