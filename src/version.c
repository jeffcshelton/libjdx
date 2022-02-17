#include "libjdx.h"

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
