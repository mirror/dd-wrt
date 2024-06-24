# JamVM + OpenJDK
> The objective of this repository is to incorporate [OpenJDK](https://openjdk.org/) and [JamVM](https://jamvm.sourceforge.net/).

JamVM is an open-source Java Virtual Machine that aims to support the
latest version of the JVM specification, while at the same time being
compact and easy to understand.

## Class Libraries

JamVM must be used in conjunction with the Java class-library from OpenJDK
to make a full Java Runtime Environment.

### OpenJDK

[OpenJDK](https://openjdk.org/) is the official open-source implementation of the JDK resulting
from the open-sourcing of Java in 2007.  It consists of a class-library,
a virtual machine (HotSpot) and a java compiler (plus other tools).

JamVM is compatible with the class-library from OpenJDK 8. JamVM provides
a shared library (`libjvm.so`) which can be used to replace the `libjvm.so`
containing HotSpot. The existing OpenJDK launcher (`java`) is used, which
now runs JamVM.

In addition to HotSpot, OpenJDK also packages and supports alternative
virtual machines such as JamVM.  To use JamVM, the `-XXaltjvm=jamvm` option
is given on the java command line.  This directs the launcher to load the
`libjvm.so` from the jamvm directory rather than the standard server or
client directory.

The easiest way to get OpenJDKis to install a pre-built package for your system.
[The Adoptium Working Group](https://adoptium.net/) supports high-quality runtimes
based on OpenJDK.  You can then replace HotSpot or the existing JamVM library
with the library you have built.

## Supported Platforms/Architectures

JamVM has been written for a Unix/POSIX-compliant system.  Unfortunately,
incompatibilities between Unixes still exist, and JamVM needs to use several
non-portable calls (mostly threading related).  In addition, some architecture
specific definitions are required (memory barriers, etc.).  Finally, the
native calling convention (or ABI) is both platform and architecture
dependent.  This is needed by JamVM when constructing a call frame to invoke
a native method.  For most platform/architectures this is provided by
highly-efficient hand-written native assembly code.

The following platforms/architectures are recognized by `configure`.
| Operating System | Microprocessor Family   |
|------------------|-------------------------|
| Linux            | x86, x86-64, Arm, Arm64 |
| FreeBSD          | x86, x86-64, Arm        |
| OpenBSD          | x86, x86-64, Arm        |
| macOS (Darwin)   | x86-64, Arm             |

## Features

For those interested in the design of virtual machines, JamVM includes a number
of optimizations to improve speed and reduce footprint.  A list, in no
particular order, is given below.
- Uses native threading (POSIX threads).  Full thread implementation including
  `Thread.interrupt()`
- Object references are direct pointers (i.e. no handles)
- Supports class loaders
- Efficient thin locks for fast locking in uncontended cases (the majority of
  locking) without using spin-locking
- Two word object header to minimize heap overhead (lock word and class pointer)
- Execution engine supports many levels of optimization (see `configure --help`)
  from basic switched interpreter to inline-threaded interpreter with stack-caching
  (aka code-copying JIT, equivalent performance to a simple JIT).
- Stop-the-world garbage collector, with separate mark/sweep and mark/compact
  phases to minimize heap fragmentation
- Thread suspension uses signals to reduce supend latency and improve performance
  (no suspension checks during normal execution)
- Full object finalisation support within the garbage collector
  (with finalizer thread)
- Full GC support for Soft, Weak and Phantom References.  References
  are enqueued using a seperate thread (the reference handler)
- Full GC support for class and class-loader unloading (including associated
  shared libraries)
- Garbage collector can run synchronously or asynchronously within its own
  thread
- String constants within class files are stored in hash table to minimize
  class data overhead (string constants shared between all classes)
- Supports JNI and dynamic loading for use with standard libraries
- Uses its own lightweight native interface for internal native methods
  without overhead of JNI 
- VM support for invokedynamic ([JSR 292](https://jcp.org/en/jsr/detail?id=292))
- VM support for type annotations ([JSR 308](https://jcp.org/en/jsr/detail?id=308))
- VM support for lambda expressions ([JSR 335](https://jcp.org/en/jsr/detail?id=335))
- VM support for method parameter reflection
- JamVM is written in C, with a small amount of platform dependent assembly
  code, and is easily portable to other architectures.

## Installation

This section describes how to configure, build and install JamVM on
your machine, along with the necessary class libraries.

### Getting the Files

For JamVM to run, it must also have a class library containing the system
classes and the associated native methods (e.g., `java.lang.Object`).
JamVM 2 has been written to work with OpenJDK.

The easiest way to get an OpenJDK install that works with JamVM is to get
an existing package for your system and then overwrite the `libjvm.so`
file with the new version.  If you wish to build OpenJDK from source,
please refer to the build instructions contained within the source distribution.

## Building and Installing JamVM

To build JamVM, you must first run `configure` to generate the Makefiles
specific to your machine.

For example, to configure JamVM to use OpenJDK 8, from the directory
containing this file do:
```shell
./configure 
```

This should print out some information and then say it's creating
the Makefiles.  If it fails, make sure your machine's architecture
is supported by JamVM.

Then, to build JamVM, type:
```shell
make
```

To install in the default location (/usr/local), type:
```shell
make install
```

This installs an executable of JamVM and a `libjvm.so` shared library with
debug information (`-g`).  Alternatively, to install a version with no symbols
(which is much smaller) type:
```shell
make install-strip
```

Note, you must be root to install in the default location.


### configure options

JamVM by default installs in `/usr/local/jamvm`.  You can change the
installation directory by giving configure the option :-
```
--prefix=PATH
```

JamVM supports a number of JamVM specific options to `configure`.
For full details do `./configure --help`.  Most of these are concerned
with enabling tracing (e.g. `--enable-tracelock`), or selecting which
interpreter variant to build.  Unless you want to debug JamVM, or
experiment with the interpreter these should not be needed.

### Installing JamVM into OpenJDK

When using JamVM with OpenJDK, JamVM's `libjvm.so` file must be
copied from its initial install location into the OpenJDK
installation.

The default install location puts `libjvm.so` into `/usr/local/jamvm/lib`.

OpenJDK allows JamVM as an alternative VM which is invoked using the
`-XXaltjvm=jamvm` option to java.  To install a new version of libjvm.so,
locate the jamvm directory within OpenJDK.  For example, for OpenJDK 8 on
an x86-64 Debian/Ubuntu system this is `/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/jamvm` .

If installing into OpenJDK, the default HotSpot libjvm.so can be overwritten
by JamVM's libjvm.so.  When java is executed it will now run JamVM (if
HotSpot is still required you can create a copy of OpenJDK, and overwrite
the libjvm.so in the copy).  HotSpot will be in either the client or server
directory, e.g. `<OpenJDK_PATH>/jre/lib/amd64/server`.

Do the copy (as root), e.g.:
```shell
mkdir -p /usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/jamvm
cp /usr/local/jamvm/lib/libjvm.so /usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/jamvm/
```

To test the install has been successful, run `java -version`, e.g.:
```shell
/usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java -XXaltjvm=jamvm -version
```

Reference output:
```
openjdk version "1.8.0_352"
OpenJDK Runtime Environment (build 1.8.0_352-8u352-ga-1~20.04-b08)
JamVM (build 2.0.1-devel, inline-threaded interpreter)
```

For Debian/Ubuntu Linux Aarch64, the steps are as following:
```shell
sudo mkdir -p /usr/lib/jvm/java-8-openjdk-arm64/jre/lib/aarch64/jamvm
sudo cp /usr/local/jamvm/lib/libjvm.so /usr/lib/jvm/java-8-openjdk-arm64/jre/lib/aarch64/jamvm
/usr/lib/jvm/java-8-openjdk-arm64/jre/bin/java -XXaltjvm=jamvm -version
```

Reference output:
```
openjdk version "1.8.0_352"
OpenJDK Runtime Environment (build 1.8.0_352-8u352-ga-1~20.04-b08)
JamVM (build 2.0.1-devel, inline-threaded interpreter with stack-caching)
```

## License

`jamvm` is available under GNU General Public License version 2.
Use of this source code is governed by the GNU GPL that can be found in the [COPYING](COPYING) file.

This project is developed by Robert Lougher and contributors.
