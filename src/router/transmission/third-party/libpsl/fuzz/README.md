# Fuzzers

These are fuzzers designed for use with `libFuzzer` or `afl`. They can
be used to run on Google's OSS-Fuzz (https://github.com/google/oss-fuzz/).

The convention used here is that the initial values for each parser fuzzer
are taken from the $NAME.in directory.

Crash reproducers from OSS-Fuzz are put into $NAME.repro directory for
regression testing with top dir 'make check' or 'make check-valgrind'.

The ./configure runs below are for libidn2.
To test libicu replace 'libidn2' with 'libicu', to test with
libidn replace 'libidn2' by 'libidn'.
To test without IDNA libraries replace `--enable-runtime=...` with `--disable-runtime`
and replace `--enable-builtin=...` with `--disable-builtin`.


# Running a fuzzer using clang

Use the following commands on top dir:
```
export CC=clang
export LIB_FUZZING_ENGINE="-lFuzzer -lstdc++"
export UBSAN_OPTIONS=print_stacktrace=1
export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer

# set flags for clang asan
CFLAGS="-O1 -fno-omit-frame-pointer -gline-tables-only -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -fsanitize=address,leak,nonnull-attribute -fsanitize-address-use-after-scope -fsanitize-coverage=trace-pc-guard,trace-cmp"

# set flags for clang ubsan
CFLAGS="$CFLAGS -fsanitize=bool,array-bounds,float-divide-by-zero,function,integer-divide-by-zero,return,shift,signed-integer-overflow,vla-bound,vptr,undefined,alignment,null,enum,integer,builtin,float-divide-by-zero,function,object-size,returns-nonnull-attribute,unsigned-integer-overflow,unreachable -fsanitize=fuzzer-no-link"

# unsigned-integer-overflow is not UB, so recover when we see it.
CFLAGS="$CFLAGS -fno-sanitize-recover=all -fsanitize-recover=unsigned-integer-overflow"

export CFLAGS

./configure --enable-static --disable-gtk-doc --enable-fuzzing --enable-runtime=libidn2 --enable-builtin=libidn2
make -j$(nproc)
cd fuzz

# run libpsl_fuzzer
./run-clang.sh libpsl_fuzzer
```


If you see a crash, then a crash corpora is written that can be used for further
investigation. E.g.
```
==2410==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000004e90 at pc 0x00000049cf9c bp 0x7fffb5543f70 sp 0x7ff
fb5543720
...
Test unit written to ./crash-adc83b19e793491b1c6ea0fd8b46cd9f32e592fc
```

To reproduce the crash:
```
./libpsl_fuzzer < ./crash-adc83b19e793491b1c6ea0fd8b46cd9f32e592fc
```

You can also copy/move that file into libpsl_fuzzer.repro/
and re-build the project without fuzzing for a valgrind run, if you like that better.
Just a `./configure` and a `make check-valgrind` should reproduce it.


# Running a fuzzer using AFL

Use the following commands on top dir:

```
$ CC=afl-clang-fast ./configure --disable-gtk-doc --enable-runtime=libidn2 --enable-builtin=libidn2
$ make -j$(nproc) clean all
$ cd fuzz
$ ./run-afl.sh libpsl_fuzzer
```

# Fuzz code coverage using the corpus directories *.in/

Code coverage reports currently work best with gcc+lcov+genhtml.

In the top directory:
```
CC=gcc CFLAGS="-O0 -g" ./configure --disable-gtk-doc --enable-runtime=libidn2 --enable-builtin=libidn2
make fuzz-coverage
xdg-open lcov/index.html
```

Each fuzzer target has it's own functions to cover, e.g.
`libpsl_fuzzer` covers psl_is_public_suffix.

To work on corpora for better coverage, `cd fuzz` and use e.g.
`./view-coverage.sh libpsl_fuzzer`.


# Enhancing the testsuite for issues found

Each reproducer file should be dropped into the appropriate *.repro/
directory.


# Clang CFI instrumentation
```
CC=clang CFLAGS="-B/usr/bin/gold -O0 -fsanitize=cfi -flto -fvisibility=default -fno-sanitize-trap=all" ./configure
make clean
make
make check
```
