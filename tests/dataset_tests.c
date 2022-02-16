#include "tests.h"

#include <errno.h>
#include <string.h>

TEST_FUNC(ReadDatasetFromPath) {
	JDXDataset *dataset = JDX_AllocDataset();
	JDXError error = JDX_ReadDatasetFromPath(dataset, "./res/example.jdx");

	final_state = (
		error == JDXError_NONE &&
		JDX_CompareVersions(dataset->header->version, JDX_VERSION) == 0 &&
		dataset->header->item_count == 8
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeDataset(dataset);
}

TEST_FUNC(WriteDatasetToPath) {
	JDXError write_error = JDX_WriteDatasetToPath(example_dataset, "./res/temp.jdx");

	JDXDataset *read_dataset = JDX_AllocDataset();
	JDXError read_error = JDX_ReadDatasetFromPath(read_dataset, "./res/temp.jdx");

	size_t image_size = (
		(size_t) example_dataset->header->image_width *
		(size_t) example_dataset->header->image_height *
		(size_t) example_dataset->header->bit_depth / 8
	);

	final_state = (
		write_error == JDXError_NONE &&
		read_error == JDXError_NONE &&
		read_dataset->header->item_count == example_dataset->header->item_count &&
		JDX_CompareVersions(read_dataset->header->version, example_dataset->header->version) == 0 &&
		memcmp(read_dataset->items[0].data, example_dataset->items[0].data, image_size) == 0
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeDataset(read_dataset);
	remove("./res/temp.jdx");
}

TEST_FUNC(CopyDataset) {
	JDXDataset *copy = JDX_AllocDataset();
	JDX_CopyDataset(copy, example_dataset);

	// TODO: Replace this memcmp with a comparison of the contents of the headers
	// final_state = (
	// 	memcmp(&example_dataset->header, &copy->header, sizeof(JDXHeader)) == 0
	// ) ? STATE_SUCCESS : STATE_FAILURE;

	final_state = STATE_SUCCESS;

	JDX_FreeDataset(copy);
}

TEST_FUNC(AppendDataset) {
	JDXDataset *copy = JDX_AllocDataset();
	JDX_CopyDataset(copy, example_dataset);

	JDXError error = JDX_AppendDataset(copy, example_dataset);

	final_state = (
		error == JDXError_NONE &&
		copy->header->item_count == example_dataset->header->item_count * 2
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeDataset(copy);
}
