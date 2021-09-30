#include "tests.h"

#include <errno.h>
#include <string.h>

void Test_ReadDatasetFromPath(void) {
    JDXDataset dataset = JDX_ReadDatasetFromPath("./res/example.jdx");

    did_fail = !(
        memcmp(&dataset.version, &JDX_VERSION, 3) == 0 &&
        dataset.item_count == 8 &&
        dataset.error == NULL
    );

    JDX_FreeDataset(dataset);
}

void Test_WriteDatasetToPath(void) {
    errno = 0;

    JDX_WriteDatasetToPath(example_dataset, "./res/temp.jdx");

    JDXDataset read_dataset = JDX_ReadDatasetFromPath("./res/temp.jdx");
    
    did_fail = (
        !!errno &&
        read_dataset.error == NULL &&
        read_dataset.item_count == example_dataset.item_count &&
        memcmp(&read_dataset.version, &example_dataset.version, sizeof(JDXVersion)) == 0 &&
        memcmp(read_dataset.images[0].data, example_dataset.images[0].data, example_dataset.images[0].width * example_dataset.images[0].height * example_dataset.images[0].color_type) == 0
    );

    JDX_FreeDataset(read_dataset);
    remove("./res/temp.jdx");

    did_fail = !!errno;
}