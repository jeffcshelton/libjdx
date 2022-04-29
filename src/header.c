#include "trycatch.h"
#include "libjdx.h"
#include "leio.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

const JDXVersion JDX_VERSION = { JDX_BUILD_DEV, 0, 4, 0 };

JDXHeader JDX_AllocHeader(void) {
	JDXHeader header;
	memset(&header, 0, sizeof(header));
	return header;
}

void JDX_FreeHeader(JDXHeader *header) {
	if (header == NULL) {
		return;
	}

	if (header->labels) {
		free(*header->labels);
		free(header->labels);
	}

	header->labels = NULL;
}

void JDX_CopyHeader(JDXHeader *dest, JDXHeader src) {
	JDX_FreeHeader(dest);

	dest->version = src.version;
	dest->image_width = src.image_width;
	dest->image_height = src.image_height;
	dest->bit_depth = src.bit_depth;

	size_t label_bytes = (size_t) (src.labels[src.label_count - 1] - *src.labels) + strlen(src.labels[src.label_count - 1]) + 1;
	char *label_buffer = malloc(label_bytes);

	dest->labels = malloc(src.label_count * sizeof(char **));
	memcpy(label_buffer, *src.labels, label_bytes);

	for (int l = 0; l < src.label_count; l++) {
		dest->labels[l] = label_buffer + (size_t) (src.labels[l] - *src.labels);
	}

	dest->label_count = src.label_count;
	dest->image_count = src.image_count;
}

size_t JDX_GetImageSize(JDXHeader header) {
	return (
		(size_t) header.image_width *
		(size_t) header.image_height *
		(size_t) header.bit_depth / 8
	);
}

JDXError JDX_ReadHeaderFromFile(JDXHeader *dest, FILE *file) {
	char *label_buffer = NULL;
	JDXHeader header = { .labels = NULL };

	TRY {
		char corruption_check[3];

		if (fread_le(corruption_check, sizeof(corruption_check), file) == EOF) {
			THROW(JDXError_READ_FILE);
		} else if (memcmp(corruption_check, "JDX", 3) != 0) {
			THROW(JDXError_CORRUPT_FILE);
		}

		uint32_t label_bytes;

		if (
			fread_le(&header.version.major, sizeof(header.version.major), file) == EOF ||
			fread_le(&header.version.minor, sizeof(header.version.minor), file) == EOF ||
			fread_le(&header.version.patch, sizeof(header.version.patch), file) == EOF ||
			fread_le(&header.version.build_type, sizeof(header.version.build_type), file) == EOF ||
			fread_le(&header.image_width, sizeof(header.image_width), file) == EOF ||
			fread_le(&header.image_height, sizeof(header.image_height), file) == EOF ||
			fread_le(&header.bit_depth, sizeof(header.bit_depth), file) == EOF ||
			fread_le(&label_bytes, sizeof(label_bytes), file) == EOF ||
			fread_le(&header.image_count, sizeof(header.image_count), file) == EOF
		) { THROW(JDXError_READ_FILE); }

		if ((header.bit_depth != 8 && header.bit_depth != 24 && header.bit_depth != 32) || (header.version.build_type > JDX_BUILD_RELEASE)) {
			THROW(JDXError_CORRUPT_FILE);
		}

		label_buffer = malloc(label_bytes);

		if (fread(label_buffer, 1, label_bytes, file) != label_bytes) {
			THROW(JDXError_READ_FILE);
		} else if (label_buffer[label_bytes - 1] != '\0') {
			THROW(JDXError_CORRUPT_FILE);
		}

		header.labels = malloc(label_bytes * sizeof(char *));
		header.labels[0] = label_buffer;
		header.label_count = 1;

		char *label_ptr = label_buffer;
		while (++label_ptr < label_buffer + label_bytes) {
			if (*(label_ptr - 1) == '\0') {
				header.labels[header.label_count++] = label_ptr;
			}
		}

		header.labels = realloc(header.labels, header.label_count * sizeof(char *));
	} CATCH(error) {
		free(label_buffer);
		return error;
	}

	JDX_FreeHeader(dest);
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

JDXError JDX_WriteHeaderToFile(JDXHeader header, FILE *file) {
	uint32_t label_bytes = (uint32_t) (header.labels[header.label_count - 1] - *header.labels) + strlen(header.labels[header.label_count - 1]) + 1;

	// Must write this way to account for alignment of JDXHeader
	if (
		fwrite("JDX", 1, 3, file) != 3 ||
		fwrite_le(&header.version.major, sizeof(header.version.major), file) == EOF ||
		fwrite_le(&header.version.minor, sizeof(header.version.minor), file) == EOF ||
		fwrite_le(&header.version.patch, sizeof(header.version.patch), file) == EOF ||
		fwrite_le(&header.version.build_type, sizeof(header.version.build_type), file) == EOF ||
		fwrite_le(&header.image_width, sizeof(header.image_width), file) == EOF ||
		fwrite_le(&header.image_height, sizeof(header.image_height), file) == EOF ||
		fwrite_le(&header.bit_depth, sizeof(header.bit_depth), file) == EOF ||
		fwrite_le(&label_bytes, sizeof(label_bytes), file) == EOF ||
		fwrite_le(&header.image_count, sizeof(header.image_count), file) == EOF ||
		fwrite(*header.labels, 1, label_bytes, file) != label_bytes ||
		fflush(file) == EOF
	) { return JDXError_WRITE_FILE; }

	return JDXError_NONE;
}
