#include "tests.h"

#include <string.h>

TEST_FUNC(CompareVersions) {
	JDXVersion less_than_current = { JDX_BUILD_DEV, 0x00, 0x00, 0x00 };
	JDXVersion greater_than_current = { JDX_BUILD_RELEASE, 0x7F, 0x7F, 0x7F };

	final_state = (
		JDX_CompareVersions(JDX_VERSION, JDX_VERSION) == 0 &&
		JDX_CompareVersions(less_than_current, JDX_VERSION) < 0 &&
		JDX_CompareVersions(greater_than_current, JDX_VERSION) > 0
	) ? STATE_SUCCESS : STATE_FAILURE;
}

TEST_FUNC(ReadHeaderFromPath) {
	JDXHeader *header = JDX_AllocHeader();
	JDXError error = JDX_ReadHeaderFromPath(header, "./res/example.jdx");

	final_state = (
		error == JDXError_NONE &&
		JDX_CompareVersions(header->version, JDX_VERSION) == 0 &&
		header->bit_depth == 24 &&
		header->image_width == 52 &&
		header->image_height == 52 &&
		header->image_count == 8
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeHeader(header);
}

TEST_FUNC(CopyHeader) {
	JDXHeader *copy = JDX_AllocHeader();
	JDX_CopyHeader(copy, example_dataset->header);

	final_state = (
		copy->image_width == example_dataset->header->image_width &&
		copy->image_height == example_dataset->header->image_height &&
		copy->bit_depth == example_dataset->header->bit_depth
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeHeader(copy);
}
