#include "tests.h"

#include <errno.h>
#include <string.h>

void Test_ReadObjectFromPath(void) {
    JDXObject obj = JDX_ReadObjectFromPath("./res/example.jdx");

    did_fail = !(
        memcmp(&obj.version, &JDX_VERSION, 3) == 0 &&
        obj.item_count == 8 &&
        obj.error == NULL
    );

    JDX_FreeObject(obj);
}

void Test_WriteObjectToPath(void) {
    errno = 0;

    JDX_WriteObjectToPath(example_obj, "./res/temp.jdx");

    JDXObject read_obj = JDX_ReadObjectFromPath("./res/temp.jdx");
    
    did_fail = (
        !!errno &&
        read_obj.error == NULL &&
        read_obj.item_count == example_obj.item_count &&
        memcmp(&read_obj.version, &example_obj.version, sizeof(JDXVersion)) == 0 &&
        memcmp(read_obj.images[0].data, example_obj.images[0].data, example_obj.images[0].width * example_obj.images[0].height * example_obj.images[0].color_type) == 0
    );

    JDX_FreeObject(read_obj);
    remove("./res/temp.jdx");

    did_fail = !!errno;
}
