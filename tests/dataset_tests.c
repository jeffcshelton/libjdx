#include "tests.h"

#include <errno.h>
#include <string.h>

void Test_ReadDatasetFromPath(void) {
    JDXDataset dataset = JDX_ReadDatasetFromPath("./res/example.jdx");

    final_state = (
        memcmp(&dataset.header.version, &JDX_VERSION, 3) == 0 &&
        dataset.header.item_count == 8 &&
        dataset.error == NULL
    ) ? STATE_SUCCESS : STATE_FAILURE;

    JDX_FreeDataset(dataset);
}

void Test_WriteDatasetToPath(void) {
    errno = 0;

    JDX_WriteDatasetToPath(example_dataset, "./res/temp.jdx");
    JDXDataset read_dataset = JDX_ReadDatasetFromPath("./res/temp.jdx");
    
    final_state = (
        !errno &&
        read_dataset.error == NULL &&
        read_dataset.header.item_count == example_dataset.header.item_count &&
        memcmp(&read_dataset.header.version, &example_dataset.header.version, sizeof(JDXVersion)) == 0 &&
        memcmp(
            read_dataset.images[0].data,
            example_dataset.images[0].data,
            example_dataset.header.image_width *
            example_dataset.header.image_height *
            example_dataset.header.color_type
        ) == 0
    ) ? STATE_SUCCESS : STATE_FAILURE;

    JDX_FreeDataset(read_dataset);
    remove("./res/temp.jdx");
}

void Test_CopyDataset(void) {
    JDXDataset copy = JDX_CopyDataset(example_dataset);

    final_state = (
        memcmp(&example_dataset.header, &copy.header, sizeof(JDXHeader)) == 0
    ) ? STATE_SUCCESS : STATE_FAILURE;

    JDX_FreeDataset(copy);
}
