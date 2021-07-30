#include "tests.h"

#include <string.h>

void Test_ReadHeaderFromPath(void) {
    JDXHeader header = JDX_ReadHeaderFromPath("./res/example.jdx");

    did_fail = !(
        memcmp(&header.version, &JDX_VERSION, 3) == 0 &&
        header.color_type == JDXColorType_RGB &&
        header.image_width == 1698 &&
        header.image_height == 2200 &&
        header.image_count == 6 &&
        header.error == NULL
    );
}
