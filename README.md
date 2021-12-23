# CImage

A fast dynamic image compression class written in C++ to compress image to the lowest possible size without loss in psychovisual quality and save in JPEG format.

## Required libraries

- [CMAKE](https://cmake.org/install/)
- [ImageMagick library](https://imagemagick.org/Magick++/Install.html) (_Check your system installation from internet_ )

## Supported formats

- JPG
- PNG
- TIF

_Any format supported by [ImageMagick formats](https://imagemagick.org/script/formats.php) should work._

## CMAKE instructions

```bash
mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
make
```

## How to run binary

- Using local iimage

  ```bash
  ./CImage ./demo.png
  ```

- Downloading image directly from url

  ```bash
  ./CImage http://some-domain.tld/image.png
  ```

- Setting output image

  ```bash
  ./CImage http://some-domain.tld/image.png output.jpg
  ```
