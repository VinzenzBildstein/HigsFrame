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

## Running HigsFrame

To run HigsFrame you need to write a helper that defines the branches read, the histograms that will be created, and the logic on how to fill these histograms.
An example can be found in the examples directory.

Command line arguments understood by Higsframe are:

| Flag         | Arguments                               | needed or optional |
| ------------ | --------------------------------------- | ------------------ |
|--input       | input root-file(s)                      | needed             |
|--helper      | datahelper source file                  | needed             |
|--calibration | calibration text file                   | optional           |
|--output      | output root-file                        | optional           |
|--tree-name   | name of root tree                       | optional           |
|--max-workers | maximum number of threads               | optional           |
|--debug       | no argument, enables debugging messages | optional           |

The calibration file is expected to be a simple ASCII file using `#` as first character for comment lines, and otherwise simply pairs of offset and gain for each detector.
The last two rows are assumed to be the time calibration and the timestamp calibration.

Running that example helper would involve a call like this
```
HigsFrame --input root_data_130Te-130Xe_run014.bin_tree.root --helper examples/ExampleHelper.cxx --max-workers 4 --calibration examples/April2025.cal
```

This example run took about 10 minutes to process the 5 GB input file using 4 threads.
Note that the processing speed can vary based on the complexity of the helper, as well as the speed of the computer.

## Helpers

Each helper has three main functions:
- the `CreateHistograms` function which is run once per worker at the beginning,
- the `Exec` function which is run for each entry of the input tree,
- the `EndOfSort` function which is run once per worker at the end.
`
There are two files for each helper:
- the header file, which
  - defines what branches are read and what the types of those branches are (in the `Book` function),
  - declares what the arguments for the `Exec` function are (the types of all the branches read), and
  - optionally declares and defines private members of the helper to store results from the `CreateHistograms` function to be used in the `Exec` function.
- the source file, which defines the three functions of the helper:
  - `CreateHistograms` is run once for each worker at the beginning and is used to define the histograms.
    Each histogram has a string that is used to find it, this is typically the same as the name of the histogram, but it doesn't have to be.
    Currently supported are histograms of type `TH1`, `TH2`, or `TH3`, as well as general `TObject`s, and `TCutG` cuts.
    The `slot` parameter passed to this function can be used to identfy the worker, e.g. to only write information to stdout if the slot is zero, i.e. the first worker.
  - `Exec` is run for each entry of the input tree and is used to fill the histograms.
    Here it is advisable to use `.at(string)` instead of `[string]` to fill the histogram tied to the key `string`, as this will produce proper exceptions if the key is not found in the map, e.g. due to a typo.
  - `EndOfSort` is an optional function (can be left blank), that is executed once per worker at the end.
    This function can e.g. be used to subtract a time-random histogram from a prompt histogram to create a time-random corrected histogram.

