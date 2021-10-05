# libjdx

libjdx is a compact library written in C that makes dealing with JDX files simple. Along with its companion [command line tool](https://github.com/jeffreycshelton/jdx-clt), libjdx can tremendously speed up development when managing large datasets of classified images, like in tasks involving machine learning, by condensing an entire dataset into one file. This file and its contained images can then be stored, loaded, or modified all at once, without the hassle of loading thousands of images and their labels individually.

## Install

You can compile and install libjdx from source by cloning this repository, navigating to its directory in a terminal, and typing:

`$ make install`

or on Windows in the Visual Studio Developer Command Prompt:

`$ nmake /f Makefile.win`

This will compile the static library for libjdx and place it in `/usr/local/lib` as well as copy the header into `/usr/local/include/libjdx.h` on Linux and macOS. On Windows, there is no install target, so the compiled static library is placed in the `lib` directory, and from there must be placed manually. To use the library in your project, you must set the library search path and include path accordingly using the compiler flags `-L/usr/local/lib` and `-I/usr/local/include` when compiling your project. On many machines, these paths are searched by default, so this step may not be necessary depending on your project environment.

**NOTE:** *libjdx requires [libdeflate](https://github.com/ebiggers/libdeflate), which included as a submodule and should be compiled correctly without any extra steps along with libjdx. If you experience any issues that you believe arise from libdeflate, please [submit an issue](https://github.com/ebiggers/libdeflate/issues) on the libdeflate repository. However, if you experience an issue that you believe to be the cause of libjdx, please [submit an issue](https://github.com/jeffreycshelton/libjdx/issues) on this repository.*

## Usage

To read a dataset from a JDX file and iterate through the images and labels within it:

```c
#include <libjdx.h>

int main(void) {
    JDXDataset dataset = JDX_ReadDatasetFromPath("path/to/file.jdx");

    if (dataset.error) {
        // Handle possible error here
    }

    for (int i = 0; i < dataset.header.item_count; i++) {
        // Access each image and its corresponding label
        JDXImage image = dataset.images[i];
        JDXLabel label = dataset.labels[i];

        // Get width and height of image
        int16_t width = image.width;
        int16_t height = image.height;

        // (int) image.color_type corresponds to the number of bytes in each pixel:
        // - JDXColorType_GRAY => 1 byte per pixel (luma)
        // - JDXColorType_RGB => 3 bytes per pixel (red, green, blue)
        // - JDXColorType_RGBA => 4 bytes per pixel (red, green, blue, alpha)

        // Get underlying image data
        uint8_t *image_data = image.data;
    }

    // Must free dataset at end of use to prevent memory leaks
    JDX_FreeDataset(dataset);
}
```

To read only the header of a JDX file:

```c
#include <jdx/jdx.h>

int main(void) {
    JDXHeader header = JDX_ReadHeaderFromPath("path/to/file.jdx");

    if (header.error) {
        // Handle possible error here
    }

    // From the header, you can access:
    // 1) JDX version of the file (header.version)
    // 2) Color type of the images (header.color_type)
    // 3) Width and height of the images (header.image_width, header.image_height)
    // 4) Number of images/labels (header.item_count)
    // 5) Size of the compressed body of the file (header.compressed_size)

    // Unlike JDXDatasets, JDXHeaders do not need to be freed
}
```

## Development

Like the other JDX tools and the format itself, libjdx is in alpha and under constant development. Please check back frequently for updates and releases that improve or patch libjdx. Contribution is also welcome! If you enjoy using libjdx or the [JDX CLT](https://github.com/jeffreycshelton/jdx-clt) and have an idea or implementation for a new feature or bug fix, please file an issue or pull request and make JDX better for everyone!

## License

libjdx is licensed under the [MIT License](LICENSE).