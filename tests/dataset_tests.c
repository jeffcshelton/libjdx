#include "tests.h"

#include <errno.h>
#include <string.h>

TEST_FUNC(ReadDatasetFromPath) {
	JDXDataset dataset = JDX_AllocDataset();
	JDXError error = JDX_ReadDatasetFromPath(&dataset, "./res/example.jdx");

	final_state = (
		error == JDXError_NONE &&
		JDX_CompareVersions(dataset.header.version, JDX_VERSION) == 0 &&
		dataset.header.image_count == 8
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeDataset(&dataset);
}

TEST_FUNC(WriteDatasetToPath) {
	JDXError write_error = JDX_WriteDatasetToPath(example_dataset, "./res/temp.jdx");

	JDXDataset read_dataset = JDX_AllocDataset();
	JDXError read_error = JDX_ReadDatasetFromPath(&read_dataset, "./res/temp.jdx");

	size_t image_block_size = JDX_GetImageSize(read_dataset.header) * read_dataset.header.image_count;
	size_t label_block_size = sizeof(JDXLabel) * read_dataset.header.image_count;

	final_state = (
		write_error == JDXError_NONE
		&& read_error == JDXError_NONE
		&& read_dataset.header.image_count == example_dataset.header.image_count
		&& JDX_CompareVersions(read_dataset.header.version, example_dataset.header.version) == 0
		&& memcmp(read_dataset._raw_image_data, example_dataset._raw_image_data, image_block_size) == 0
		&& memcmp(read_dataset._raw_labels, example_dataset._raw_labels, label_block_size) == 0
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeDataset(&read_dataset);
	remove("./res/temp.jdx");
}

TEST_FUNC(CopyDataset) {
	JDXDataset copy = JDX_AllocDataset();
	JDX_CopyDataset(&copy, example_dataset);

	size_t image_block_size = JDX_GetImageSize(copy.header) * copy.header.image_count;
	size_t label_block_size = sizeof(JDXLabel) * copy.header.image_count;

	final_state = (
		copy.header.image_count == example_dataset.header.image_count
		&& memcmp(copy._raw_image_data, example_dataset._raw_image_data, image_block_size) == 0
		&& memcmp(copy._raw_labels, example_dataset._raw_labels, label_block_size) == 0
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeDataset(&copy);
}

TEST_FUNC(AppendDataset) {
	JDXDataset copy = JDX_AllocDataset();
	JDX_CopyDataset(&copy, example_dataset);

	JDXError error = JDX_AppendDataset(&copy, example_dataset);

	size_t image_block_size = JDX_GetImageSize(example_dataset.header) * example_dataset.header.image_count;
	size_t label_block_size = sizeof(JDXLabel) * example_dataset.header.image_count;

	final_state = (
		error == JDXError_NONE
		&& copy.header.image_count == example_dataset.header.image_count * 2
		&& memcmp(example_dataset._raw_image_data, copy._raw_image_data + image_block_size, image_block_size) == 0
		&& memcmp(example_dataset._raw_labels, (uint8_t *) copy._raw_labels + label_block_size, label_block_size) == 0
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeDataset(&copy);
}
