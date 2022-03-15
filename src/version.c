#include "libjdx.h"

int32_t JDX_CompareVersions(JDXVersion v1, JDXVersion v2) {
	return v1.raw - v2.raw;
}
