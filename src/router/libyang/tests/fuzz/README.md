# FUZZING
This directory contains a collection of fuzz harnesses, which are designed to
be used with [AFL](http://lcamtuf.coredump.cx/afl/) and [LibFuzzer](https://llvm.org/docs/LibFuzzer.html)
fuzzers. The harnesses should also be easily reusable with other similar fuzzers.

Two asciinema examples are available, one for LibFuzzer:
https://asciinema.org/a/311035
and one for AFL:
https://asciinema.org/a/311060

To build the fuzz targets, the ENABLE_FUZZ_TARGETS option has to be enabled.
The FUZZER option specifies which fuzzer to use, currently only AFL and LibFuzzer
are supported, with AFL being the default. LibFuzzer is based on the same 
principles that AFL works with, but has a different implementation of the fuzzing engine
and is integrated with UBSAN by default, while AFL lacks official integration with UBSAN.

To use the harnesses with AFL, one of AFL's compilers should be used.
For example the AFL clang-fast compiler can be used with the cmake option shown below.
It is recommended to set the build type to Release, since otherwise the fuzzer will
detect failed asserts as crashes.
If LibFuzzer is used, clang has to be used, as gcc doesn't support -fsanitize=fuzzer.

```
$ cmake -DENABLE_FUZZ_TARGETS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=path_to_afl/afl-clang-fast ..
```

After that the programs can be built by running make:
```
$ make
```

The target executables will then be available in tests/fuzz of the build directory that was used.

The libyang yang test files available in the `tests/modules/` subdirectory can be used as initial
test cases for fuzzing targets that receive YANG models, like the lys_parse_mem_fuzz_harness. However, a smaller corpus of YANG models should probably
be used, as larger models decrease execution speed. A good place to start would be to collect
small YANG files, each of which uses only a single YANG feature.

The files that will be used as starting test cases should be copied into a single directory. Those files should then be minimized by using afl-cmin and afl-tmin.

To increase the speed of fuzzing, the test cases and AFL output files should be stored on a temporary RAM disk.
If a new fuzz target is used, AFL persistent mode should be used. More about persistent mode can be read in the official AFL documentation.

When all of the above is done the fuzzing process can begin. AFL supports running multiple instances of the fuzzer, which can speed up the
process on multi core CPUs. The first fuzz instance should be started in master mode, and the other instances in slave mode.
The total number of instances should usually be equal to the number of cores.

Below is an example of running 2 instances. The -i flag specifies the testcase input directory, and the -o file specifies the directory the fuzzer will use for output.
```
afl-fuzz -i minimised_testcases/ -o syncdir/ -M fuzzer1 -- libyang/build/tests/fuzz/lyd_parse_mem_fuzz_harness
afl-fuzz -i minimised_testcases/ -o syncdir/ -S fuzzer2 -- libyang/build/tests/fuzz/lyd_parse_mem_fuzz_harness
```

To fuzz with LibFuzzer, at the most basic level, everything that is required is
to run the compiled fuzz target.
However, running the target like that invokes the harness with only one job
on a single core, with no starting inputs. 
Multiple jobs running on separate cores should be used, with a starting input corpus.
The options are described in the official LibFuzzer documentation (https://llvm.org/docs/LibFuzzer.html).

## Fuzzing corpus and regression testing
The `tests/fuzz/corpus` directory contains subdirectories for every fuzz target. Those subdirectories contain a collection of previous inputs that were found by fuzzing and caused visible issues or crashes. Every input file is named after the issue or pull request where it was originally reported. When a new issue is discovered, the input causing the issue should be added to the appropriate directory.

These input files are then used by the fuzz_regression_test test which sends the corpus into the corresponding fuzz harness, to test whether any of the files crash and cause regressions.
