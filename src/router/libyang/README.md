# libyang

[![BSD license](https://img.shields.io/badge/License-BSD-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Build Status](https://secure.travis-ci.org/CESNET/libyang.png?branch=master)](http://travis-ci.org/CESNET/libyang/branches)
[![codecov.io](https://codecov.io/github/CESNET/libyang/coverage.svg?branch=master)](https://codecov.io/github/CESNET/libyang?branch=master)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/5259/badge.svg)](https://scan.coverity.com/projects/5259)
[![Ohloh Project Status](https://www.openhub.net/p/libyang/widgets/project_thin_badge.gif)](https://www.openhub.net/p/libyang)

libyang is a YANG data modelling language parser and toolkit written (and
providing API) in C. The library is used e.g. in [libnetconf2](https://github.com/CESNET/libnetconf2),
[Netopeer2](https://github.com/CESNET/Netopeer2), [sysrepo](https://github.com/sysrepo/sysrepo) and
[FRRouting](https://github.com/frrouting/frr) projects.

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
* [yanglint](#yanglint) - feature-rich YANG tool.

Current implementation covers YANG 1.0 ([RFC 6020](https://tools.ietf.org/html/rfc6020))
as well as YANG 1.1 ([RFC 7950](https://tools.ietf.org/html/rfc7950)).

## Branches

The project uses 2 main branches `master` and `devel`. Other branches should not be cloned. In `master` there are files of the
last official *release*. Any latest improvements and changes, which were tested at least briefly are found in `devel`. On every
new *release*, `devel` is merged into `master`.

This means that when only stable official releases are to be used, either `master` can be used or specific *releases* downloaded.
If all the latest bugfixes should be applied, `devel` branch is the  one to be used. Note that whenever **a new issue is created**
and it occurs on the `master` branch, the **first response will likely be** to use `devel` before any further provided support.

## Packages

We are using openSUSE Build Service to automaticaly prepare binary packages for number of GNU/Linux distros.
The [libyang](https://software.opensuse.org//download.html?project=home%3Aliberouter&package=libyang)
packages are always build from current `master` branch (latest release). If you are interested in any other packages
(such as *devel* or C++ and Python bindings), you can browse
[all packages](https://download.opensuse.org/repositories/home:/liberouter/) from our repository.

## Requirements

### Build Requirements

* C compiler (gcc >= 4.8.4, clang >= 3.0, ...)
* cmake >= 2.8.12
* libpcre (devel package)
 * note, that PCRE is supposed to be compiled with unicode support (configure's options
   `--enable-utf` and `--enable-unicode-properties`)
* cmocka >= 1.0.0 (for tests only, see [Tests](#Tests))

#### Optional

* doxygen (for generating documentation)
* valgrind (for enhanced testing)

### Runtime Requirements

* libpcre

## Building

```
$ mkdir build; cd build
$ cmake ..
$ make
# make install
```

### Documentation

The library documentation can be generated directly from the source codes using
Doxygen tool:
```
$ make doc
$ google-chrome ../doc/html/index.html
```

The documentation is also built hourly and available at
[netopeer.liberouter.org](https://netopeer.liberouter.org/doc/libyang/master/).

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

#### Optimizations

Whenever the latest revision of a schema is supposed to be loaded (import without specific revision),
it is performed in the standard way, the first time. By default, every other time when the latest
revision of the same schema is needed, the one initially loaded is reused. If you know this can cause
problems meaning the latest available revision of a schema can change during operation, you can force
libyang to always search for the schema anew by:

```
$ cmake -DENABLE_LATEST_REVISIONS=OFF ..
```

Also, it can be efficient to store certain information about schemas that is generated during parsing
so that it does not need to be generated every time the schema is used, but it will consume some
additional space. You can enable this cache with:

```
$ cmake -DENABLE_CACHE=ON ..
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

## yanglint

libyang project includes a feature-rich tool called `yanglint(1)` for validation
and conversion of the schemas and YANG modeled data. The source codes are
located at [`/tools/lint`](./tools/lint) and can be used to explore how an
application is supposed to use the libyang library. `yanglint(1)` binary as
well as its man page are installed together with the library itself.

There is also [README](./tools/lint/examples/README.md) describing some examples of
using `yanglint`.

libyang supports YANG extensions via a plugin mechanism. Some of the plugins (for
NACM or Metadata) are available out of the box and installed together with libyang.
However, when libyang is not installed and `yanglint(1)` is used from the build
directory, the plugins are not available. There are two options:

1. Install libyang.
```
# make install
```

2. Set environment variable `LIBYANG_EXTENSIONS_PLUGINS_DIR` to contain path to the
   built extensions plugin (`./src/extensions` from the build directory).
```
$ LIBYANG_EXTENSIONS_PLUGINS_DIR="`pwd`/src/extensions" ./yanglint
```

## Tests

libyang includes several tests built with [cmocka](https://cmocka.org/). The tests
can be found in `tests` subdirectory and they are designed for checking library
functionality after code changes.

The tests are by default built in the `Debug` build mode by running
```
$ make
```

In case of the `Release` mode, the tests are not built by default (it requires
additional dependency), but they can be enabled via cmake option:
```
$ cmake -DENABLE_BUILD_TESTS=ON ..
```

Note that if the necessary [cmocka](https://cmocka.org/) headers are not present
in the system include paths, tests are not available despite the build mode or
cmake's options.

Tests can be run by the make's `test` target:
```
$ make test
```

## Fuzzing

Simple fuzzing targets, fuzzing instructions and a Dockerfile that builds the fuzz targets
and the AFL fuzzer are available in the `tests/fuzz` directory.

The `tests/fuzz` directory also contains a README file that describes the whole process in more detail.

## Bindings

We provide bindings for high-level languages using [SWIG](http://www.swig.org/)
generator. The bindings are optional and to enable building of the specific
binding, the appropriate cmake option must be enabled, for example:
```
$ cmake -DJAVASCRIPT_BINDING=ON ..
```

More information about the specific binding can be found in their README files.

Currently supported bindings are:

* JavaScript
    - cmake option: `JAVASCRIPT_BINDING`
    - [README](./swig/javascript/README.md)
* Python SWIG (uses SWIG, enabled by default if `GEN_LANGUAGE_BINDINGS` is set)
    - cmake option: `GEN_PYTHON_BINDINGS` (depends on `GEN_CPP_BINDINGS`)
    - [README](./swig/python/README.md)
* Python CFFI (more "pythonic" API)
    - Hosted in a separate project: https://github.com/CESNET/libyang-python

## Project Information

Project is hosted on [GitHub](https://github.com/CESNET/libyang) where you can find additional information and contact developers via the project's issue tracker. If you are interested in future plans announcements, please subscribe to the [Future Plans issue](https://github.com/CESNET/libyang/issues/880).
