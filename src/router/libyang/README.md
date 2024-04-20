# libyang

[![BSD license](https://img.shields.io/badge/License-BSD-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Build](https://github.com/CESNET/libyang/workflows/libyang%20CI/badge.svg)](https://github.com/CESNET/libyang/actions?query=workflow%3A%22libyang+CI%22)
[![Docs](https://img.shields.io/badge/docs-link-blue)](https://netopeer.liberouter.org/doc/libyang/)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/5259/badge.svg)](https://scan.coverity.com/projects/5259)
[![codecov.io](https://codecov.io/github/CESNET/libyang/coverage.svg?branch=master)](https://codecov.io/github/CESNET/libyang?branch=master)
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/libyang.svg)](https://bugs.chromium.org/p/oss-fuzz/issues/list?sort=-opened&can=1&q=proj:libyang)
[![Ohloh Project Status](https://www.openhub.net/p/libyang/widgets/project_thin_badge.gif)](https://www.openhub.net/p/libyang)

libyang is a YANG data modelling language parser and toolkit written (and
providing API) in C. The library is used e.g. in [libnetconf2](https://github.com/CESNET/libnetconf2),
[Netopeer2](https://github.com/CESNET/Netopeer2) or [sysrepo](https://github.com/sysrepo/sysrepo) projects.

## Branches

The project uses 2 main branches `master` and `devel`. Other branches should not be cloned. In `master` there are files of the
last official *release*. Any latest improvements and changes, which were tested at least briefly are found in `devel`. On every
new *release*, `devel` is merged into `master`.

This means that when only stable official releases are to be used, either `master` can be used or specific *releases* downloaded.
If all the latest bugfixes should be applied, `devel` branch is the  one to be used. Note that whenever **a new issue is created**
and it occurs on the `master` branch, the **first response will likely be** to use `devel` before any further provided support.

## Migration from libyang version 1 or older

Look into the documentation and the section `Transition Manual`. That should help with basic migration and the
ability to compile a project. But to actually make use of the new features, it is required to read through
the whole documentation and the API.

## Provided Features

* Parsing (and validating) schemas in YANG format.
* Parsing (and validating) schemas in YIN format.
* Parsing, validating and printing instance data in XML format.
* Parsing, validating and printing instance data in JSON format
  ([RFC 7951](https://tools.ietf.org/html/rfc7951)).
* Manipulation with the instance data.
* Support for default values in the instance data ([RFC 6243](https://tools.ietf.org/html/rfc6243)).
* Support for YANG extensions.
* Support for YANG Metadata ([RFC 7952](https://tools.ietf.org/html/rfc7952)).
* Support for YANG Schema Mount ([RFC 8528](https://tools.ietf.org/html/rfc8528)).
* Support for YANG Structure ([RFC 8791](https://tools.ietf.org/html/rfc8791)).
* [yanglint](#yanglint) - feature-rich YANG tool.

Current implementation covers YANG 1.0 ([RFC 6020](https://tools.ietf.org/html/rfc6020))
as well as YANG 1.1 ([RFC 7950](https://tools.ietf.org/html/rfc7950)).

## Packages

Binary RPM or DEB packages of the latest release can be built locally using `apkg`, look into `README` in
the `distro` directory.

## Requirements

### Unix Build Requirements

* C compiler
* cmake >= 2.8.12
* libpcre2 >= 10.21 (including devel package)
  * note, that PCRE is supposed to be compiled with unicode support (configure's options
    `--enable-utf` and `--enable-unicode-properties`)

#### Optional

* doxygen (for generating documentation)
* cmocka >= 1.0.1 (for [tests](#Tests))
* valgrind (for enhanced testing)
* gcov (for code coverage)
* lcov (for code coverage)
* genhtml (for code coverage)

### Unix Runtime Requirements

* libpcre2 >= 10.21

### Windows Build Requirements

* Visual Studio 17 (2022)
* cmake >= 3.22.0
* libpcre2 (same considerations as on POSIX)
* [`pthreads-win32`](https://sourceware.org/pthreads-win32/)
* [`dirent`](https://github.com/tronkko/dirent)
* [`dlfcn-win32`](https://github.com/dlfcn-win32/dlfcn-win32)
* [`getopt-win32`](https://github.com/libimobiledevice-win32/getopt)

The Windows version [does not support plugins](https://github.com/CESNET/libyang/commit/323c31221645052e13db83f7d0e6e51c3ce9d802), and the `yanglint` works in a [non-interactive mode](https://github.com/CESNET/libyang/commit/2e3f935ed6f4a47e65b31de5aeebcd8877d5a09b) only.
On Windows, all YANG date-and-time values are first converted to UTC (if TZ offset was specified), and then returned with "unspecified timezone".

## Building

```
$ mkdir build; cd build
$ cmake ..
$ make
# make install
```

### Useful CMake Options

#### Changing Compiler

Set `CC` variable:

```
$ CC=/usr/bin/clang cmake ..
```

#### Changing Install Path

To change the prefix where the library, headers and any other files are installed,
set `CMAKE_INSTALL_PREFIX` variable:
```
$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
```

Default prefix is `/usr/local`.

#### Build Modes

There are two build modes:
* Release.
  This generates library for the production use without any debug information.
* Debug.
  This generates library with the debug information and disables optimization
  of the code.

The `Debug` mode is currently used as the default one. to switch to the
`Release` mode, enter at the command line:
```
$ cmake -D CMAKE_BUILD_TYPE:String="Release" ..
```

#### Changing Extensions Plugins Directory

As for YANG extensions, libyang allows loading extension plugins. By default, the
directory to store the plugins is LIBDIR/libyang. To change it, use the following
cmake option with the value specifying the desired directory:

```
$ cmake -DPLUGINS_DIR:PATH=`pwd`"/src/extensions/" ..
```

The directory path can be also changed runtime via environment variable, e.g.:

```
$ LIBYANG_EXTENSIONS_PLUGINS_DIR=`pwd`/my/relative/path yanglint
```

Note that plugins are [not available on Windows](https://github.com/CESNET/libyang/commit/323c31221645052e13db83f7d0e6e51c3ce9d802).

#### Optimizations

Whenever the latest revision of a schema is supposed to be loaded (import without specific revision),
it is performed in the standard way, the first time. By default, every other time when the latest
revision of the same schema is needed, the one initially loaded is reused. If you know this can cause
problems meaning the latest available revision of a schema can change during operation, you can force
libyang to always search for the schema anew by:

```
$ cmake -DENABLE_LATEST_REVISIONS=OFF ..
```

### CMake Notes

Note that, with CMake, if you want to change the compiler or its options after
you already ran CMake, you need to clear its cache first - the most simple way
to do it is to remove all content from the 'build' directory.

## Usage

All libyang functions are available via the main header:
```
#include <libyang/libyang.h>
```

To compile your program with libyang, it is necessary to link it with libyang using the
following linker parameters:
```
-lyang
```

Note, that it may be necessary to call `ldconfig(8)` after library installation and if the
library was installed into a non-standard path, the path to libyang must be specified to the
linker. To help with setting all the compiler's options, there is `libyang.pc` file for
`pkg-config(1)` available in the source tree. The file is installed with the library.

If you are using `cmake` in you project, it is also possible to use the provided
`FindLibYANG.cmake` file to detect presence of the libyang library in the system.

## Bindings

There are no bindings for other languages directly in this project but they are
available separately.

* [Python](https://github.com/CESNET/libyang-python/)
* [C++](https://github.com/CESNET/libyang-cpp/)
* [Rust](https://github.com/rwestphal/yang2-rs/)

## yanglint

libyang project includes a feature-rich tool called `yanglint(1)` for validation
and conversion of the schemas and YANG modeled data. The source codes are
located at [`/tools/lint`](./tools/lint) and can be used to explore how an
application is supposed to use the libyang library. `yanglint(1)` binary as
well as its man page are installed together with the library itself.

There is also [README](./tools/lint/examples/README.md) describing some examples of
using `yanglint`.

## Tests

libyang includes several tests built with [cmocka](https://cmocka.org/). The tests
can be found in `tests` subdirectory and they are designed for checking library
functionality after code changes. Additional regression tests done with
a corpus of fuzzing inputs that previously caused crashes are done.
Those are available in `tests/fuzz` and are built automatically with the
cmocka unit tests.


The tests are by default built in the `Debug` build mode by running
```
$ make
```

In case of the `Release` mode, the tests are not built by default (it requires
additional dependency), but they can be enabled via cmake option:
```
$ cmake -DENABLE_TESTS=ON ..
```

Note that if the necessary [cmocka](https://cmocka.org/) headers are not present
in the system include paths, tests are not available despite the build mode or
cmake's options.

Tests can be run by the make's `test` target:
```
$ make test
```

### Perf

There is a performance measurement tool included that prints information about
the time required to execute common use-cases of working with YANG instance data.

To enable this test, use an option and to get representative results, enable Release build type:
```
$ cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_PERF_TESTS=ON ..
```
and to run the test with seeing its output run:
```
$ make
$ ctest -V -R ly_perf
```

### Code Coverage

Based on the tests run, it is possible to generate code coverage report. But
it must be enabled and these commands are needed to generate the report:
```
$ cmake -DENABLE_COVERAGE=ON ..
$ make
$ make coverage
```

## Fuzzing

Multiple YANG fuzzing targets and fuzzing instructions are available in the
`tests/fuzz` directory.

All of the targets can be fuzzed with LLVM's LibFuzzer and AFL, and new targets
can easily be added.
Asciinema examples which describe the fuzzing setup for both AFL (https://asciinema.org/a/311060)
and LibFuzzer (https://asciinema.org/a/311035) are available.
