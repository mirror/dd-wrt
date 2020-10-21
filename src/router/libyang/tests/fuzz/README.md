# FUZZING
Two simple programs, xmlfuzz and yangfuzz are available in this directory and
are designed to be used with the [AFL](http://lcamtuf.coredump.cx/afl/) fuzzer.

To build the fuzz targets, the ENABLE_BUILD_FUZZ_TARGETS option has to be enabled.

To add AFL instrumentation when compiling the programs, the AFL clang-fast compiler
should be used with the following cmake option:

It is recommended to set the build type to Release, since otherwise the fuzzer will detect failed asserts as crashes.

```
$ cmake -DENABLE_BUILD_FUZZ_TARGETS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=path_to_afl/afl-clang-fast ..
```

After that the programs can be built by running make:
```
$ make
```

The yangfuzz and xmlfuzz executables will then be available in the root of the build directory that was used.

The libyang yang and xml test files available in the `tests/data/files` subdirectory can be used as initial
test cases for fuzzing the programs with AFL.

The files that will be used as starting test cases should be copied into a single directory. Those files should then be minimized by using afl-cmin and afl-tmin.
Also, AFL will issue a warning about a decrease of performance when working with large files, so only smaller test cases should be used.

To increase the speed of fuzzing, the test cases and AFL output files should be stored on a temporary RAM disk.
If a new fuzz target is used, AFL persistent mode should be used. More about persistent mode can be read in the official AFL documentation.

When all of the above is done the fuzzing process can begin. AFL supports running multiple instances of the fuzzer, which can speed up the
process on multi core CPUs. The first fuzz instance should be started in master mode, and the other instances in slave mode.
The total number of instances should usually be equal to the number of cores.

Below is an example of running 2 instances. The -i flag specifies the testcase input directory, and the -o file specifies the directory the fuzzer will use for output.
```
afl-fuzz -i minimised_testcases/ -o syncdir/ -M fuzzer1 -- libyang/build/fuzz/yangfuzz @@
afl-fuzz -i minimised_testcases/ -o syncdir/ -S fuzzer2 -- libyang/build/fuzz/yangfuzz @@
```

# Dockerfile

Since setting up the fuzz targets can be a complicated process, a Dockerfile that builds libyang with the fuzzing targets is also available in this directory, and can be used to fuzz libyang.
