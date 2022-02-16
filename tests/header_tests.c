#include "tests.h"

#include <string.h>

TEST_FUNC(CompareVersions) {
	JDXVersion less_than_current = { 0, 0, 0, JDXBuildType_BETA };
	JDXVersion greater_than_current = { 255, 255, 255, JDXBuildType_RELEASE };

	final_state = (
		JDX_CompareVersions(JDX_VERSION, JDX_VERSION) == 0 &&
		JDX_CompareVersions(less_than_current, JDX_VERSION) == -1 &&
		JDX_CompareVersions(greater_than_current, JDX_VERSION) == 1
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
		header->item_count == 8
	) ? STATE_SUCCESS : STATE_FAILURE;

	JDX_FreeHeader(header);
}
