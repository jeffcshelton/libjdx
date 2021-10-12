#include "libjdx.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

const JDXVersion JDX_VERSION = { 0, 1, 0 };

JDXError JDX_ReadHeaderFromFile(JDXHeader *dest, FILE *file) {
    char corruption_check[3];

    if (fread(corruption_check, 1, sizeof(corruption_check), file) != sizeof(corruption_check))
        return JDXError_READ_FILE;

    if (memcmp(corruption_check, "JDX", 3) != 0)
        return JDXError_CORRUPT_FILE;

    // Must read this way to account for alignment of JDXHeader
    JDXHeader header;
    if (
        fread(&header.version, sizeof(header.version), 1, file) != 1 ||
        fread(&header.image_width, sizeof(header.image_width), 1, file) != 1 ||
        fread(&header.image_height, sizeof(header.image_height), 1, file) != 1 ||
        fread(&header.bit_depth, sizeof(header.bit_depth), 1, file) != 1 ||
        fread(&header.item_count, sizeof(header.item_count), 1, file) != 1 ||
        fread(&header.compressed_size, sizeof(header.compressed_size), 1, file) != 1
    ) return JDXError_READ_FILE;

    if (header.bit_depth != 8 && header.bit_depth != 24 && header.bit_depth != 32)
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
    
    // Must write this way to account for alignment of JDXHeader
    if (
        fwrite(&header.version, sizeof(header.version), 1, file) != 1 ||
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
