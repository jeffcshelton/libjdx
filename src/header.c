#include "libjdx.h"
#include "leio.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

JDXHeader *JDX_AllocHeader(void) {
	return calloc(1, sizeof(JDXHeader));
}

static inline void free_header_labels(JDXHeader *header) {
	if (header->labels) {
		if (header->label_count > 0) {
			for (uint_fast16_t l = 0; l < header->label_count; l++) {
				free((char *) header->labels[l]);
			}
		}

		free(header->labels);
	}
}

void JDX_FreeHeader(JDXHeader *header) {
	if (header == NULL) {
		return;
	}

	free_header_labels(header);
	free(header);
}

void JDX_CopyHeader(JDXHeader *dest, JDXHeader *src) {
	dest->version = src->version;
	dest->image_width = src->image_width;
	dest->image_height = src->image_height;
	dest->bit_depth = src->bit_depth;

	free_header_labels(dest);
	dest->labels = malloc(src->label_count * sizeof(char **));

	for (uint_fast16_t l = 0; l < src->label_count; l++) {
		size_t label_size = strlen(src->labels[l]) + 1;

		dest->labels[l] = malloc(label_size);
		memcpy((char *) dest->labels[l], src->labels[l], label_size);
	}

	dest->label_count = src->label_count;

	dest->item_count = src->item_count;
	dest->compressed_size = src->compressed_size;
}

JDXError JDX_ReadHeaderFromFile(JDXHeader *dest, FILE *file) {
	char corruption_check[3];

	if (fread(corruption_check, 1, sizeof(corruption_check), file) != sizeof(corruption_check))
		return JDXError_READ_FILE;

	if (memcmp(corruption_check, "JDX", 3) != 0)
		return JDXError_CORRUPT_FILE;

	if (
		fread_le(&dest->version.major, sizeof(dest->version.major), file) == EOF ||
		fread_le(&dest->version.minor, sizeof(dest->version.minor), file) == EOF ||
		fread_le(&dest->version.patch, sizeof(dest->version.patch), file) == EOF ||
		fread_le(&dest->version.build_type, sizeof(dest->version.build_type), file) == EOF ||
		fread_le(&dest->image_width, sizeof(dest->image_width), file) == EOF ||
		fread_le(&dest->image_height, sizeof(dest->image_height), file) == EOF ||
		fread_le(&dest->bit_depth, sizeof(dest->bit_depth), file) == EOF ||
		fread_le(&dest->label_count, sizeof(dest->label_count), file) == EOF
	) return JDXError_READ_FILE;

	char **labels = malloc(dest->label_count * sizeof(char *));

	char buf[JDX_MAX_LABEL_LEN];
	int i;

	for (int_fast16_t l = 0; l < dest->label_count; l++) {
		i = 0;
		while (i < JDX_MAX_LABEL_LEN && (buf[i++] = getc(file)));

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

	dest->labels = (const char **) labels;

	if (
		fread_le(&dest->item_count, sizeof(dest->item_count), file) == EOF ||
		fread_le(&dest->compressed_size, sizeof(dest->compressed_size), file) == EOF
	) {
		free_header_labels(dest);
		return JDXError_READ_FILE;
	}

	if ((dest->bit_depth != 8 && dest->bit_depth != 24 && dest->bit_depth != 32) || (dest->version.build_type > JDX_BUILD_RELEASE)) {
		free_header_labels(dest);
		return JDXError_CORRUPT_FILE;
	}

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

JDXError JDX_WriteHeaderToFile(JDXHeader *header, FILE *file) {
	char corruption_check[3] = {'J', 'D', 'X'};

	if (fwrite(corruption_check, 1, sizeof(corruption_check), file) != sizeof(corruption_check))
		return JDXError_WRITE_FILE;

	// Must write this way to account for alignment of JDXHeader
	if (
		fwrite_le(&header->version.major, sizeof(header->version.major), file) == EOF ||
		fwrite_le(&header->version.minor, sizeof(header->version.minor), file) == EOF ||
		fwrite_le(&header->version.patch, sizeof(header->version.patch), file) == EOF ||
		fwrite_le(&header->version.build_type, sizeof(header->version.build_type), file) == EOF ||
		fwrite_le(&header->image_width, sizeof(header->image_width), file) == EOF ||
		fwrite_le(&header->image_height, sizeof(header->image_height), file) == EOF ||
		fwrite_le(&header->bit_depth, sizeof(header->bit_depth), file) == EOF ||
		fwrite_le(&header->label_count, sizeof(header->label_count), file) == EOF
	) return JDXError_WRITE_FILE;

	const char *label;
	int len;

	for (int_fast16_t l = 0; l < header->label_count; l++) {
		label = header->labels[l];
		len = strlen(label) + 1;

		if (fwrite(label, sizeof(char), len, file) != len) {
			return JDXError_WRITE_FILE;
		}
	}

	if (
		fwrite_le(&header->item_count, sizeof(header->item_count), file) == EOF ||
		fwrite_le(&header->compressed_size, sizeof(header->compressed_size), file) == EOF
	) return JDXError_WRITE_FILE;

	if (fflush(file) == EOF)
		return JDXError_WRITE_FILE;

	return JDXError_NONE;
}
