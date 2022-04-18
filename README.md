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
    JDXDataset *dataset = JDX_AllocDataset();
    JDXError read_error = JDX_ReadDatasetFromPath(dataset, "path/to/file.jdx");

    if (read_error) {
        // Handle possible error here.
    }

    for (uint64_t i = 0; i < dataset.header.image_count; i++) {
        // Access each image:
        JDXImage *image = JDX_GetImage(dataset, i);

        // Get width, height, and bit depth of image:
        uint16_t width = image->width;
        uint16_t height = image->height;
        uint8_t bit_depth = image->bit_depth;

        // Each possible value of image->bit_depth corresponds to the number of bits in each pixel:
        // - 8 bits => 1 bytes per pixel (luma)
        // - 24 bits => 3 bytes per pixel (red, green, blue)
        // - 32 bits => 4 bytes per pixel (red, green, blue, alpha)

        // Get label of image (both the string label and numberical label):
        const char *label = image->label;
        uint16_t label_num = image->label_index;

        // Get underlying pixel data:
        uint8_t *pixel_data = image->raw_data;

        // Must free image at the end of use because a new one is copied for each call of JDX_GetImage.
        JDX_FreeImage(image);
    }

    // Must free dataset at end of use to prevent memory leaks.
    JDX_FreeDataset(dataset);
}
```

To read only the header of a JDX file:

```c
#include <libjdx.h>

int main(void) {
    JDXHeader *header = JDX_AllocHeader();
    JDXError read_error = JDX_ReadHeaderFromPath(header, "path/to/file.jdx");

    if (read_error) {
        // Handle possible error here.
    }

    // From the header, you can access:
    // 1) JDX version of the file (header.version)
    // 2) Width and height of the images (header.image_width, header.image_height)
    // 3) Bit depth of the images (header.bit_depth)
    // 4) Number of images (header.image_count)
    // 5) Stringified labels (header.labels w/ header.label_count)

    // Like JDXDatasets, JDXHeaders also need to be freed to avoid memory leaks.
    JDX_FreeHeader(header);
}
```

## Development

Like the other JDX tools and the format itself, libjdx is in alpha and under constant development. Please check back frequently for updates and releases that improve or patch libjdx. Contribution is also welcome! If you enjoy using libjdx or the [JDX CLT](https://github.com/jeffreycshelton/jdx-clt) and have an idea or implementation for a new feature or bug fix, please file an issue or pull request and make JDX better for everyone!

## License

libjdx is licensed under the [MIT License](LICENSE).