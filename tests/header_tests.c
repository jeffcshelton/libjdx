#include "tests.h"

#include <string.h>

void Test_ReadHeaderFromPath(void) {
    JDXHeader header = JDX_ReadHeaderFromPath("./res/example.jdx");

    did_fail = !(
        memcmp(&header.version, &JDX_VERSION, 3) == 0 &&
        header.color_type == JDXColorType_RGB &&
        header.image_width == 52 &&
        header.image_height == 52 &&
        header.image_count == 8 &&
        header.error == NULL
    );
}
