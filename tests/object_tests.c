#include "tests.h"

#include <string.h>

void Test_ReadObjectFromPath(void) {
    JDXObject obj = JDX_ReadObjectFromPath("./res/example.jdx");

    did_fail = !(
        memcmp(&obj.version, &JDX_VERSION, 3) == 0 &&
        obj.item_count == 6 &&
        obj.error == NULL
    );

    JDX_FreeObject(obj);
}
