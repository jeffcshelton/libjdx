#include "libjdx.h"

const JDXVersion JDX_VERSION = {{ JDX_BUILD_DEV, 0, 4, 0 }};

int32_t JDX_CompareVersions(JDXVersion v1, JDXVersion v2) {
	return v1.raw - v2.raw;
}
