# The `ttf2pkf` tool

A tool that converts most of the font formats (those supported by the FreeType library) to the internal kernel PKF
format.

This tool is not limited to convert from ttf files nor from bitmap files. It is able to convert from many more
(scalable or not) font files. However, note that the PKF format is quite limited and therefore
many scalable fonts may not render quite well in the kernel.

## Documentation

Usage: `ttf2pkf path/to/font.ttf -o path/to/font.pkf -s 16`

Options:

- `-o filename`: specify the output PKF file path
- `-s size`: specify the font size in pixels
- `-c++`: specify to generate a C++ file with a static array storing the PKF file instead of a raw PKF file.

## How to build

The project uses CMake. Therefore, it can be build using the following commands:

```
cmake -S . -B build
make -j -C build
```

Note that the library FreeType is required.
