# HigsFrame

A wrapper around RDataFrame to allow quick processing of root-trees with data from HIGS.

## Compiling HigsFrame

This project uses cmake.
One way to install it is to run
`
cmake -S . -B build
cmake --build build
`

Note: For macOS, it is necessary to add `-DCMAKE_PREFIX_PATH=/opt/local/libexec/` to the first line.

