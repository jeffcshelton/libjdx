#include "trycatch.h"
#include "libjdx.h"
#include "leio.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libdeflate.h>

// TODO: For creating, copying, and appending JDXDataset consider using block memory allocation instead of many mallocs

JDXDataset *JDX_AllocDataset(void) {
	return calloc(1, sizeof(JDXDataset));
}

void JDX_FreeDataset(JDXDataset *dataset) {
	if (dataset == NULL) {
		return;
	}

	JDX_FreeHeader(dataset->header);
	free(dataset->_raw_image_data);
	free(dataset->_raw_labels);
	free(dataset);
}

void JDX_CopyDataset(JDXDataset *dest, const JDXDataset *src) {
	dest->header = JDX_AllocHeader();
	JDX_CopyHeader(dest->header, src->header);

	size_t image_block_size = JDX_GetImageSize(src->header) * (size_t) src->header->image_count;

	size_t label_block_size = (
		(size_t) src->header->image_count *
		sizeof(uint16_t)
	);

	dest->_raw_labels = malloc(label_block_size);
	memcpy(dest->_raw_labels, src->_raw_labels, label_block_size);

	dest->_raw_image_data = malloc(image_block_size);
	memcpy(dest->_raw_image_data, src->_raw_image_data, image_block_size);
}

JDXError JDX_AppendDataset(JDXDataset *dest, const JDXDataset *src) {
	// Check for any compatibility errors between the two datasets
	if (src->header->image_width != dest->header->image_width) {
		return JDXError_UNEQUAL_WIDTHS;
	} else if (src->header->image_height != dest->header->image_height) {
		return JDXError_UNEQUAL_HEIGHTS;
	} else if (src->header->bit_depth != dest->header->bit_depth) {
		return JDXError_UNEQUAL_BIT_DEPTHS;
	}

	uint16_t src_label_map[src->header->label_count];

	uint_fast16_t max_label_count = dest->header->label_count + src->header->label_count;
	dest->header->labels = realloc(dest->header->labels, max_label_count * sizeof(char *));
	uint_fast16_t label_count = dest->header->label_count;

	for (uint_fast16_t i = 0; i < src->header->label_count; i++) {
		uint_fast16_t j;

		for (j = 0; j < dest->header->label_count; j++) {
			if (strcmp(src->header->labels[i], dest->header->labels[j]) == 0) {
				src_label_map[i] = j;
			}
		}

		if (j != dest->header->label_count) {
			uint_fast16_t l = label_count++;

			dest->header->labels[l] = strdup(src->header->labels[i]);
			src_label_map[i] = l;
		}
	}

	// Reallocate smaller to prevent wasting space for large sets of labels
	if (label_count < max_label_count) {
		dest->header->labels = realloc(dest->header->labels, label_count * sizeof(char *));
	}

	dest->header->label_count = label_count;

	// Calculate final item count and realloc destination arrays accordingly
	uint64_t new_image_count = dest->header->image_count + src->header->image_count;
	size_t image_size = (
		(size_t) src->header->image_width *
		(size_t) src->header->image_height *
		(size_t) src->header->bit_depth / 8
	);

	dest->_raw_labels = realloc(dest->_raw_labels, new_image_count * sizeof(uint16_t));

	dest->_raw_image_data = realloc(dest->_raw_image_data, image_size * (size_t) new_image_count);
	memcpy(
		dest->_raw_image_data + image_size * (size_t) dest->header->image_count,
		src->_raw_image_data,
		image_size * (size_t) src->header->image_count
	);

	for (uint_fast64_t s = 0, d = dest->header->image_count; s < src->header->image_count; s++, d++) {
		dest->_raw_labels[d] = src_label_map[src->_raw_labels[s]];
	}

	dest->header->image_count = new_image_count;

	return JDXError_NONE;
}

JDXError JDX_ReadDatasetFromFile(JDXDataset *dest, FILE *file) {
	// Declare all allocated pointers so that they can easily be freed in the event of an error
	struct libdeflate_decompressor *decompressor = NULL;
	uint8_t *decompressed_body = NULL;
	uint8_t *compressed_body = NULL;
	uint8_t *raw_image_data = NULL;
	uint16_t *raw_labels = NULL;
	JDXHeader *header = NULL;

	TRY {
		header = JDX_AllocHeader();
		JDXError header_error = JDX_ReadHeaderFromFile(header, file);

		if (header_error) {
			THROW(header_error);
		}

		uint64_t compressed_size;
		if (fread(&compressed_size, sizeof(compressed_size), 1, file) != 1) {
			THROW(JDXError_READ_FILE);
		}

		compressed_body = malloc((size_t) compressed_size);

		if (fread(compressed_body, 1, compressed_size, file) != compressed_size) {
			THROW(JDXError_READ_FILE);
		}

		size_t image_size = (
			(size_t) header->image_width *
			(size_t) header->image_height *
			(size_t) header->bit_depth / 8
		);

		size_t decompressed_size = (image_size + sizeof(uint16_t)) * (size_t) header->image_count;
		decompressed_body = malloc(decompressed_size);

		// Decompress encoded body
		decompressor = libdeflate_alloc_decompressor();
		enum libdeflate_result decompress_result = libdeflate_deflate_decompress(
			decompressor, compressed_body, compressed_size,
			decompressed_body, decompressed_size, NULL
		);

		if (decompress_result != LIBDEFLATE_SUCCESS) {
			THROW(JDXError_CORRUPT_FILE);
		}

		raw_image_data = malloc(image_size * header->image_count);
		raw_labels = malloc(header->image_count * sizeof(uint16_t));

		uint8_t *dest_chunk_ptr = raw_image_data;
		uint8_t *src_chunk_ptr = decompressed_body;

		for (int i = 0; i < header->image_count; i++) {
			memcpy(dest_chunk_ptr, src_chunk_ptr, image_size);
			dest_chunk_ptr += image_size;
			src_chunk_ptr += image_size;

			raw_labels[i] = *((uint16_t *) src_chunk_ptr);
			src_chunk_ptr += sizeof(uint16_t);
		}
	} CATCH(error) {
		libdeflate_free_decompressor(decompressor);
		free(decompressed_body);
		free(compressed_body);

		JDX_FreeHeader(header);
		return error;
	}

	libdeflate_free_decompressor(decompressor);
	free(decompressed_body);
	free(compressed_body);

	JDX_FreeHeader(dest->header);
	free(dest->_raw_image_data);
	free(dest->_raw_labels);

	dest->header = header;
	dest->_raw_image_data = raw_image_data;
	dest->_raw_labels = raw_labels;

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
	// Declare all allocated pointers so that they can easily be freed in the event of an error
	struct libdeflate_compressor *compressor = NULL;
	uint8_t *uncompressed_body = NULL;
	uint8_t *compressed_body = NULL;

	TRY {
		size_t image_size = (
			(size_t) dataset->header->image_width *
			(size_t) dataset->header->image_height *
			(size_t) dataset->header->bit_depth / 8
		);

		size_t uncompressed_size = (
			(size_t) (image_size + sizeof(uint16_t)) *
			(size_t) dataset->header->image_count
		);

		uncompressed_body = malloc(uncompressed_size);

		uint8_t *body_ptr = uncompressed_body;
		uint8_t *image_data_ptr = dataset->_raw_image_data;

		for (int i = 0; i < dataset->header->image_count; i++) {
			memcpy(body_ptr, image_data_ptr, image_size);
			image_data_ptr += image_size;
			body_ptr += image_size;

			*((uint16_t *) body_ptr) = dataset->_raw_labels[i];
			body_ptr += sizeof(uint16_t);
		}

		// Must allocate for entire uncompressed size despite it almost certainly being less
		compressed_body = malloc(uncompressed_size);
		
		compressor = libdeflate_alloc_compressor(12);
		size_t compressed_size = libdeflate_deflate_compress(
			compressor,
			uncompressed_body,
			uncompressed_size,
			compressed_body,
			uncompressed_size
		);

		// libdeflate will return 0 if operation failed
		if (compressed_size == 0) {
			THROW(JDXError_WRITE_FILE);
		}

		JDXError header_error = JDX_WriteHeaderToFile(dataset->header, file);

		if (header_error) {
			THROW(header_error);
		}

		if (
			fwrite(&compressed_size, sizeof(compressed_size), 1, file) != 1 ||
			fwrite(compressed_body, 1, compressed_size, file) != compressed_size ||
			fflush(file) == EOF
		) {
			THROW(JDXError_WRITE_FILE);
		}
	} CATCH(error) {
		libdeflate_free_compressor(compressor);
		free(uncompressed_body);
		free(compressed_body);

		return error;
	}

	libdeflate_free_compressor(compressor);
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
