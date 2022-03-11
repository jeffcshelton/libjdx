#include "trycatch.h"
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
}

JDXError JDX_ReadHeaderFromFile(JDXHeader *dest, FILE *file) {
	char corruption_check[3];
	char label_buffer[JDX_MAX_LABEL_LEN];
	JDXHeader header = { .labels = NULL };

	TRY {
		if (fread_le(corruption_check, sizeof(corruption_check), file) == EOF) {
			THROW(JDXError_READ_FILE);
		} else if (memcmp(corruption_check, "JDX", 3) != 0) {
			THROW(JDXError_CORRUPT_FILE);
		}

		if (
			fread_le(&header.version.major, sizeof(header.version.major), file) == EOF ||
			fread_le(&header.version.minor, sizeof(header.version.minor), file) == EOF ||
			fread_le(&header.version.patch, sizeof(header.version.patch), file) == EOF ||
			fread_le(&header.version.build_type, sizeof(header.version.build_type), file) == EOF ||
			fread_le(&header.image_width, sizeof(header.image_width), file) == EOF ||
			fread_le(&header.image_height, sizeof(header.image_height), file) == EOF ||
			fread_le(&header.bit_depth, sizeof(header.bit_depth), file) == EOF ||
			fread_le(&header.label_count, sizeof(header.label_count), file) == EOF
		) { THROW(JDXError_READ_FILE); }

		// TODO: Consider doing this with only 2 mallocs; it may speed it up
		header.labels = calloc(header.label_count, sizeof(char *));

		for (int_fast16_t l = 0; l < header.label_count; l++) {
			int i = 0;
			while (i < JDX_MAX_LABEL_LEN && (label_buffer[i++] = getc(file)));

			if (label_buffer[i - 1]) {
				THROW(JDXError_CORRUPT_FILE);
			}

			header.labels[l] = malloc(i);
			memcpy((char *) header.labels[l], label_buffer, i);
		}

		if (fread_le(&header.item_count, sizeof(header.item_count), file) == EOF) {
			THROW(JDXError_READ_FILE);
		}

		if ((header.bit_depth != 8 && header.bit_depth != 24 && header.bit_depth != 32) || (header.version.build_type > JDX_BUILD_RELEASE)) {
			THROW(JDXError_CORRUPT_FILE);
		}
	} CATCH(error) {
		if (header.labels) {
			for (int_fast16_t l = 0; l < header.label_count; l++) {
				free((char *) header.labels[l]);
			}

			free(header.labels);
		}

		return error;
	}

	if (dest && dest->labels) {
		free_header_labels(dest);
	}

	*dest = header;
	return JDXError_NONE;
}

JDXError JDX_ReadHeaderFromPath(JDXHeader *dest, const char *path) {
	FILE *file = fopen(path, "rb");

	if (file == NULL) {
		return JDXError_OPEN_FILE;
	}

	JDXError error = JDX_ReadHeaderFromFile(dest, file); // Named 'error' but could (and should) be 'JDXError_NONE'

	if (fclose(file) == EOF) {
		return JDXError_CLOSE_FILE;
	}

	return error;
}

JDXError JDX_WriteHeaderToFile(JDXHeader *header, FILE *file) {
	char corruption_check[3] = {'J', 'D', 'X'};

	if (fwrite(corruption_check, 1, sizeof(corruption_check), file) != sizeof(corruption_check)) {
		return JDXError_WRITE_FILE;
	}

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
	) { return JDXError_WRITE_FILE; }

	for (int_fast16_t l = 0; l < header->label_count; l++) {
		char *label = (char *) header->labels[l];
		int len = strlen(label) + 1;

		if (fwrite_le(label, len, file) == EOF) {
			return JDXError_WRITE_FILE;
		}
	}

	if (
		fwrite_le(&header->item_count, sizeof(header->item_count), file) == EOF ||
		fflush(file) == EOF
	) { return JDXError_WRITE_FILE; }

	return JDXError_NONE;
}
