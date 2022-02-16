#include "libjdx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libdeflate.h>

// TODO: For creating, copying, and appending JDXDataset consider using block memory allocation instead of many mallocs

JDXDataset *JDX_AllocDataset(void) {
	return calloc(1, sizeof(JDXDataset));
}

JDXError JDX_ReadDatasetFromFile(JDXDataset *dest, FILE *file) {
	if (dest->header) {
		JDX_FreeHeader(dest->header);
	}

	dest->header = JDX_AllocHeader();
	JDXError header_error = JDX_ReadHeaderFromFile(dest->header, file);

	if (header_error != JDXError_NONE) {
		return header_error;
	}

	size_t compressed_size = (size_t) dest->header->compressed_size;
	uint8_t *compressed_body = malloc(compressed_size);

	if (fread(compressed_body, 1, compressed_size, file) != compressed_size) {
		free(compressed_body);
		return JDXError_READ_FILE;
	}

	size_t image_size = (
		(size_t) dest->header->image_width *
		(size_t) dest->header->image_height *
		(size_t) dest->header->bit_depth / 8
	);

	size_t decompressed_size = (image_size + sizeof(JDXLabel)) * (size_t) dest->header->item_count;
	uint8_t *decompressed_body = malloc(decompressed_size);

	// Decompress encoded body and free the decompressor
	struct libdeflate_decompressor *decompressor = libdeflate_alloc_decompressor();
	enum libdeflate_result decompress_result = libdeflate_deflate_decompress(
		decompressor, compressed_body, compressed_size,
		decompressed_body, decompressed_size, NULL
	);

	libdeflate_free_decompressor(decompressor);

	if (decompress_result != LIBDEFLATE_SUCCESS) {
		free(compressed_body);
		free(decompressed_body);
		return JDXError_CORRUPT_FILE;
	}

	dest->items = malloc(dest->header->item_count * sizeof(JDXItem));

	uint8_t *chunk_ptr = decompressed_body;
	for (int i = 0; i < dest->header->item_count; i++) {
		// Allocate and copy image data into new image buffer and advance chunk ptr
		uint8_t *image_data = malloc(image_size);
		memcpy(image_data, chunk_ptr, image_size);
		chunk_ptr += image_size;

		// Type pun end of chunk into label and advance chunk ptr again
		JDXLabel label = *((JDXLabel *) chunk_ptr);
		chunk_ptr += sizeof(JDXLabel);

		JDXItem item = {
			image_data,
			dest->header->image_width,
			dest->header->image_height,
			dest->header->bit_depth,
			label
		};

		dest->items[i] = item;
	}

	// Free compressed and decompressed body
	free(compressed_body);
	free(decompressed_body);

	return JDXError_NONE;
}

JDXError JDX_ReadDatasetFromPath(JDXDataset *dest, const char *path) {
	FILE *file = fopen(path, "rb");

	if (file == NULL) {
		return JDXError_OPEN_FILE;
	}

	JDXError error = JDX_ReadDatasetFromFile(dest, file); // Named 'error' but could (and should) be 'JDXError_NONE'

	if (fclose(file) == EOF) {
		return JDXError_CLOSE_FILE;
	}

	return error;
}

JDXError JDX_WriteDatasetToFile(JDXDataset *dataset, FILE *file) {
	size_t image_size = (
		(size_t) dataset->header->image_width *
		(size_t) dataset->header->image_height *
		(size_t) dataset->header->bit_depth / 8
	);

	size_t uncompressed_size = (
		(size_t) (image_size + sizeof(JDXLabel)) *
		(size_t) dataset->header->item_count
	);

	uint8_t *uncompressed_body = malloc(uncompressed_size);
	uint8_t *body_ptr = uncompressed_body;

	for (int i = 0; i < dataset->header->item_count; i++) {
		memcpy(body_ptr, dataset->items[i].data, image_size);
		body_ptr += image_size;

		*((JDXLabel *) body_ptr) = dataset->items[i].label;
		body_ptr += sizeof(JDXLabel);
	}

	// Must allocate for entire uncompressed size despite it almost certainly being less
	uint8_t *compressed_body = malloc(uncompressed_size);
	
	struct libdeflate_compressor *compressor = libdeflate_alloc_compressor(12);
	size_t compressed_size = libdeflate_deflate_compress(
		compressor,
		uncompressed_body,
		uncompressed_size,
		compressed_body,
		uncompressed_size
	);

	libdeflate_free_compressor(compressor);

	// libdeflate will return 0 if operation failed
	if (compressed_size == 0) {
		free(uncompressed_body);
		free(compressed_body);

		return JDXError_WRITE_FILE;
	}

	dataset->header->compressed_size = (uint64_t) compressed_size;
	JDXError header_error = JDX_WriteHeaderToFile(dataset->header, file);

	if (header_error != JDXError_NONE) {
		free(uncompressed_body);
		free(compressed_body);

		return header_error;
	}
		

	if (fwrite(compressed_body, 1, compressed_size, file) != compressed_size || fflush(file) == EOF) {
		free(uncompressed_body);
		free(compressed_body);

		return JDXError_WRITE_FILE;
	}

	free(uncompressed_body);
	free(compressed_body);

	return JDXError_NONE;
}

JDXError JDX_WriteDatasetToPath(JDXDataset *dataset, const char *path) {
	FILE *file = fopen(path, "wb");

	if (file == NULL) {
		return JDXError_OPEN_FILE;
	}

	JDXError error = JDX_WriteDatasetToFile(dataset, file);
	
	if (fclose(file) == EOF) {
		return JDXError_CLOSE_FILE;
	}

	return error;
}

void JDX_CopyDataset(JDXDataset *dest, JDXDataset *src) {
	dest->header = JDX_AllocHeader();
	JDX_CopyHeader(dest->header, src->header);

	dest->items = malloc(src->header->item_count * sizeof(JDXItem));

	size_t image_size = (
		(size_t) src->header->image_width *
		(size_t) src->header->image_height *
		(size_t) src->header->bit_depth / 8
	);

	for (int i = 0; i < src->header->item_count; i++) {
		JDXItem item_copy = {
			malloc(image_size),
			src->header->image_width,
			src->header->image_height,
			src->header->bit_depth,
			src->items[i].label
		};

		memcpy(item_copy.data, src->items[i].data, image_size);
		dest->items[i] = item_copy;
	}
}

JDXError JDX_AppendDataset(JDXDataset* dest, JDXDataset *src) {
	// Check for any compatibility errors between the two datasets
	if (src->header->image_width != dest->header->image_width) {
		return JDXError_UNEQUAL_WIDTHS;
	} else if (src->header->image_height != dest->header->image_height) {
		return JDXError_UNEQUAL_HEIGHTS;
	} else if (src->header->bit_depth != dest->header->bit_depth) {
		return JDXError_UNEQUAL_BIT_DEPTHS;
	}

	// Calculate final item count and realloc destination arrays accordingly
	uint64_t new_item_count = dest->header->item_count + src->header->item_count;
	dest->items = realloc(dest->items, new_item_count * sizeof(JDXItem));

	size_t image_size = (
		(size_t) src->header->image_width *
		(size_t) src->header->image_height *
		(size_t) src->header->bit_depth / 8
	);

	// Copy each image and label individually and store them in dest
	for (int s = 0, d = dest->header->item_count; s < src->header->item_count; s++, d++) {
		JDXItem copy_item = {
			malloc(image_size),
			src->header->image_width,
			src->header->image_height,
			src->header->bit_depth,
			src->items[s].label
		};

		memcpy(copy_item.data, src->items[s].data, image_size);
		dest->items[d] = copy_item;
	}

	// Set destination item count
	dest->header->item_count = new_item_count;

	return JDXError_NONE;
}

void JDX_FreeDataset(JDXDataset *dataset) {
	for (int i = 0; i < dataset->header->item_count; i++) {
		free(dataset->items[i].data);
	}

	JDX_FreeHeader(dataset->header);
	free(dataset->items);
	free(dataset);
}
