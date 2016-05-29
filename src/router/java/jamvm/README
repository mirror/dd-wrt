JamVM 2.0.0
===========

Welcome to the twenty-eighth release of JamVM!

JamVM is an open-source Java Virtual Machine that aims to support the
latest version of the JVM specification, while at the same time being
compact and easy to understand.

Class Libraries
===============

JamVM must be used in conjunction with a Java class-library to make
a full Java Runtime Environment.  Compatible class-libraries are GNU
Classpath or the class-library from OpenJDK.

GNU Classpath
-------------

GNU Classpath is a community-driven reimplementation of the Java
class-libraries.  For many years it was the only library supported by
JamVM.  Since the open-sourcing of Java, development of GNU Classpath
has slowed but it is still active and maintained.  The latest release
is 0.99, released on 16th March 2012.

When configured for GNU Classpath, JamVM provides a shared library
(libjvm.so) for use with the JNI Invocation API, and a standalone
executable (jamvm) to run Java programs.

GNU Classpath is mostly Java 1.5 compliant, with some APIs from Java 1.6.
Although JamVM supports Java 7 and Java 8, these features are disabled
when used with GNU Classpath.  GNU Classpath, however, is much smaller
than OpenJDK, which makes it suitable for embedded systems.

By default, if no class-library is specified when configuring JamVM,
JamVM is built to use GNU Classpath. 

OpenJDK/IcedTea
---------------

OpenJDK is the official open-source implementation of the JDK resulting
from the open-sourcing of Java in 2007.  It consists of a class-library,
a virtual machine (HotSpot) and a java compiler (plus other tools).

JamVM is compatible with the class-library from OpenJDK 6, 7 and 8 (the
latest).  When configured for use with OpenJDK, JamVM provides a
shared library (libjvm.so) which can be used to replace the libjvm.so
containing HotSpot.  The existing OpenJDK launcher (java) is used, which
now runs JamVM.

IcedTea is a build harness for OpenJDK that initially replaced encumbered
parts of OpenJDK with free components.  In addition to HotSpot, IcedTea
also packages and supports alternative virtual machines such as JamVM.  To
use JamVM, the -jamvm option is given on the java command line.  This
directs the launcher to load the libjvm.so from the jamvm directory rather
than the standard server or client directory.

The easiest way to get OpenJDK/IcedTea is to install a pre-built package
for your system.  You can then replace HotSpot or the existing JamVM library
with the library you have built.

Supported Platforms/Architectures
=================================

JamVM has been written for a Unix/Posix-compliant system.  Unfortunately,
incompatibilities between Unixes still exist, and JamVM needs to use several
non-portable calls (mostly threading related).  In addition, some architecture
specific definitions are required (memory barriers, etc.).  Finally, the
native calling convention (or ABI) is both platform and architecture
dependent.  This is needed by JamVM when constructing a call frame to invoke
a native method.  For most platform/architectures this is provided by
highly-efficient hand-written native assembler, although libffi is also
supported for all platforms (specified by --with-libffi when configuring).
Libffi is less efficient than the hand-written assembler although recent
versions of JamVM also includes stubs for common method signatures.

The following platforms/architectures are recognised by configure.  Those
marked with * must be configured to use libffi.

- Linux: x86, x86_64, ARM, PowerPC, PowerPC64(*), MIPS, HPPA
- FreeBSD: x86, x86_64, ARM, PowerPC, PowerPC64(*), SPARC(*)
- OpenBSD: x86, x86_64, ARM, PowerPC, PowerPC64(*), SPARC(*)
- Mac OS X/Darwin: x86, x86_64, ARM, PowerPC, PowerPC64
- Solaris/OpenSolaris: x86, x86_64
- KFreeBSD: x86

JamVM "Features"
================

For those interested in the design of virtual machines, JamVM includes a number
of optimisations to improve speed and reduce foot-print.  A list, in no
particular order, is given below.

- Uses native threading (posix threads).  Full thread implementation
  including Thread.interrupt()

- Object references are direct pointers (i.e. no handles)

- Supports class loaders

- Efficient thin locks for fast locking in uncontended cases (the
  majority of locking) without using spin-locking

- Two word object header to minimise heap overhead (lock word and
  class pointer)

- Execution engine supports many levels of optimisation (see
  configure --help) from basic switched interpreter to inline-threaded
  interpreter with stack-caching (aka code-copying JIT, equivalent
  performance to a simple JIT).

- Stop-the-world garbage collector, with separate mark/sweep
  and mark/compact phases to minimise heap fragmentation

- Thread suspension uses signals to reduce supend latency and improve
  performance (no suspension checks during normal execution)

- Full object finalisation support within the garbage collector
  (with finaliser thread)

- Full GC support for Soft, Weak and Phantom References.  References
  are enqueued using a seperate thread (the reference handler)

- Full GC support for class and class-loader unloading (including
  associated shared libraries)

- Garbage collector can run synchronously or asynchronously within its
  own thread

- String constants within class files are stored in hash table to
  minimise class data overhead (string constants shared between all
  classes)

- Supports JNI and dynamic loading for use with standard libraries

- Uses its own lightweight native interface for internal native methods
  without overhead of JNI 

- VM support for invokedynamic (JSR 292)
- VM support for type annotations (JSR 308)
- VM support for lambda expressions (JSR 335)
- VM support for method parameter reflection

- JamVM is written in C, with a small amount of platform dependent
  assembler, and is easily portable to other architectures.


That's it!

Robert Lougher <rob@jamvm.org.uk>
30th July 2014.
