#include "libjdx.h"

int32_t JDX_CompareVersions(JDXVersion v1, JDXVersion v2) {
	return *((int32_t *) &v1) - *((int32_t *) &v2);
}
