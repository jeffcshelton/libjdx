# libjdx

libjdx is a compact library written in C that makes dealing with JDX files simple. Along with its companion [command line tool](https://github.com/jeffreycshelton/jdx-clt), libjdx can tremendously speed up development when managing large datasets of classified images, like in machine learning, by condensing the entire dataset into one file. This file and its contained images can then be stored, loaded, or modified all at once, without the hassle of loading thousands of images and their labels individually.

## Install

libjdx requires [libdeflate](https://github.com/ebiggers/libdeflate). If you are on macOS, you can install it using [Homebrew](https://brew.sh):

`$ brew install libdeflate`

On Linux, you can compile it from source by following the instructions under "For UNIX" of the [libdeflate repository](https://github.com/ebiggers/libdeflate). As of now, libjdx does not support Windows but will before it is out of alpha.

## Usage

To read from a JDX file and iterate through the images and labels within it:

```c
#include <jdx/jdx.h>

int main(void) {
    JDXObject obj = JDX_ReadObjectFromPath("path/to/file.jdx");

    if (obj.error) {
        // Handle possible error here
    }

    for (int i = 0; i < obj.item_count; i++) {
        // Access corresponding image and its label
        JDXImage image = obj.images[i];
        JDXLabel label = obj.labels[i];

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

    // Must free object at end of use to prevent memory leaks
    JDX_FreeObject(obj);
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
    // 3) Image width and height (header.image_width, header.image_height)
    // 4) Number of images (header.image_count)
    // 5) Size of the compressed body of the file (header.body_size)

    // Unlike JDXObjects, JDXHeaders do not need to be freed
}
```

## Development

Like the other JDX tools and the format itself, libjdx is in alpha and under constant development. Please check back frequently for updates and releases that improve or patch libjdx. Contribution is also welcome! If you enjoy using libjdx or the [JDX CLT](https://github.com/jeffreycshelton/jdx-clt) and have an idea or implementation for a new feature or bug fix, please file an issue or pull request and make JDX better for everyone!

## License

libjdx is licensed under the [MIT License](LICENSE)