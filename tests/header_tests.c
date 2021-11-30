#include "tests.h"

#include <string.h>

void Test_ReadHeaderFromPath(void) {
	JDXHeader header;
	JDXError error = JDX_ReadHeaderFromPath(&header, "./res/example.jdx");

	final_state = (
		error == JDXError_NONE &&
		JDX_CompareVersions(header.version, JDX_VERSION) == 0 &&
		header.bit_depth == 24 &&
		header.image_width == 52 &&
		header.image_height == 52 &&
		header.item_count == 8
	) ? STATE_SUCCESS : STATE_FAILURE;
}
