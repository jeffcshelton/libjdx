#include "libjdx.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define SIGN(x) ((x) > 0 ? 1 : -1)

const JDXVersion JDX_VERSION = { 0, 4, 0, JDXBuildType_DEV };

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
		fread(&header.version.major, sizeof(header.version.major), 1, file) != 1 ||
		fread(&header.version.minor, sizeof(header.version.minor), 1, file) != 1 ||
		fread(&header.version.patch, sizeof(header.version.patch), 1, file) != 1 ||
		fread(&build_type, sizeof(build_type), 1, file) != 1 ||
		fread(&header.image_width, sizeof(header.image_width), 1, file) != 1 ||
		fread(&header.image_height, sizeof(header.image_height), 1, file) != 1 ||
		fread(&header.bit_depth, sizeof(header.bit_depth), 1, file) != 1 ||
		fread(&header.item_count, sizeof(header.item_count), 1, file) != 1 ||
		fread(&header.compressed_size, sizeof(header.compressed_size), 1, file) != 1
	) return JDXError_READ_FILE;

	header.version.build_type = (JDXBuildType) build_type;

	if ((header.bit_depth != 8 && header.bit_depth != 24 && header.bit_depth != 32) || (header.version.build_type > JDXBuildType_RELEASE))
		return JDXError_CORRUPT_FILE;

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
		fwrite(&header.version.major, sizeof(header.version.major), 1, file) != 1 ||
		fwrite(&header.version.minor, sizeof(header.version.minor), 1, file) != 1 ||
		fwrite(&header.version.patch, sizeof(header.version.patch), 1, file) != 1 ||
		fwrite(&build_type, sizeof(build_type), 1, file) != 1 ||
		fwrite(&header.image_width, sizeof(header.image_width), 1, file) != 1 ||
		fwrite(&header.image_height, sizeof(header.image_height), 1, file) != 1 ||
		fwrite(&header.bit_depth, sizeof(header.bit_depth), 1, file) != 1 ||
		fwrite(&header.item_count, sizeof(header.item_count), 1, file) != 1 ||
		fwrite(&header.compressed_size, sizeof(header.compressed_size), 1, file) != 1
	) return JDXError_WRITE_FILE;

	if (fflush(file) == EOF)
		return JDXError_WRITE_FILE;

	return JDXError_NONE;
}
