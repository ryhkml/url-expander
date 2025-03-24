# URL Expander

A simple tool to expand shortened URLs and enhance transparency by uncovering their true destinations.

## Build and Usage

Before you run the command below, URL Expander requires [libcurl](https://curl.se/libcurl).

```sh
cc -o nob nob.c && ./nob
```

## Formatter

`.clang-format` is based on [Google](https://google.github.io/styleguide/cppguide.html) style guide

```
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 120
AlignArrayOfStructures: Left
AlignAfterOpenBracket: Align
BracedInitializerIndentWidth: 4
```
