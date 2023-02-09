Building libpsl with Visual Studio
==================================

Building libpsl for Windows using Visual Studio 2008 or later is
supported with NMake (from release tarballs) or Meson (from GIT
checkouts).  The following sections will cover building libpsl with
these methods.

Currently, for builtin/runtime public suffix list (PSL) IDNA handling,
only ICU is supported for Visual Studio builds.

Using NMake (from a release tarball)
===========
You will need a Python 2.7.x or later installation in order to
complete the build successfully.

You will need the ICU (International Components for Unicode)
libraries, headers and DLLs, to build libpsl, unless both
DISABLE_BUILTIN=1 and DISABLE_RUNTIME=1 are passed into the NMake
command line as listed below.

You can also buid libpsl with libiconv and gettext support, please
see the options below for enabling such support.

In a Visual Studio command prompt which matches your desired
configuration (x86/Win32, x64 etc.),
go to $(srcroot)\msvc, and issue the following command:

nmake /f Makefile.vc CFG=[debug|release]

A 'test' target is provided to build the test programs, while a
'clean' target is provided to remove all the compiled and generated
files for the build.  An 'install' target is provided to copy the
build PSL DLL, .lib and executables, as well as the related PDB files
and libpsl header, into appropriate locations under PREFIX (please see
below).

This will build the libpsl DLL/LIB and the psl.exe utility in the
vsX\$(CFG)\$(ARCH) subdirectory, where X is the release version
of Visual Studio, such as 9 for 2008 and 16 for 2019, and ARCH is 
Win32 for 32-bit builds and x64 for 64-bit (x86_64) builds.

A number of options can be passed into the NMake command, as follows.
Enable by setting each option to 1, unless otherwise indicated:

*  PSL_FILE: Location of the PSL data file, which is retrieved from
             https://publicsuffix.org/list/public_suffix_list.dat,
             or some other custom location (not supported).  Default
             is in $(srcroot)\list\public_suffix_list.dat.  This is
             needed to generate the suffixes_dafsa.h header required
             for the build, as well as the binary and ascii dafsa
             files used for the test programs.

*  TEST_PSL_FILE: Location of the test PSL file.  Default is in
                  $(srcroot)\list\tests\tests.txt.  This is
                  required for building and running the test
                  programs.

*  STATIC: Set if building static versions of libpsl is desired.

*  USE_LIBTOOL_DLLNAME: Set to use libtool-style DLL naming.

*  DISABLE_RUNTIME: Do not use ICU to generate runtime PSL data.

*  DISABLE_BUILTIN: Do not use ICU to generate builtin PSL data.

*  USE_ICONV: Enable libiconv support, requires libiconv.

*  USE_GETTEXT: Enable gettext support for displaying i18n messages.
                Implies USE_ICONV, and requires gettext-runtime.

*  PYTHON: Full path to a Python 2.7.x (or later) interpreter, if not
           already in your PATH.
           Required to generate DAFSA headers and data files that is
           needed for the build, as well as generating pkg-config
           files for NMake builds.

*  PREFIX: Base installation path of the build.  Note that any dependent
           libraries are searched first from the include\ and lib\
           sub-directories in PREFIX before searching in the paths
           specified by %INCLUDE% and %LIB%.  Default is
           $(srcroot)\..\vsX\$(PLATFORM), where X is the release version
           of Visual Studio, such as 9 for 2008 and 16 for 2019,
           $(PLATFORM) is the target platform (Win32/x64) of the build.

Building libpsl with Meson
==========================
Building using Meson is now supported for Visual Studio builds from a
GIT checkout.

Besides the requirements listed in the NMake builds, you will also need

*  Python 3.5.x or later
*  Meson build system, use PIP to install from Python 3.5.x64
*  Ninja build tool (if not involking Meson with --backend=
   vs[2010|2015|2017|2019])
*  A compatible PSL data file and a test PSL data file.  You may
   consider using the ones shipped with the latest libpsl release
   tarball and place the PSL data file in $(srcroot)/list and the
   test PSL data file in $(srcroot)/list/tests.  You may also choose
   to download the latest PSL data file from
   https://publicsuffix.org/list/public_suffix_list.dat and place it
   it $(srcroot)/list.  Alternatively, specify
   -Dpsl_file=<path_to_psl_data_file> and/or
   -Dpsl_testfile=<path_to_test_psl_data_file> when invoking Meson.

Open a Visual Studio command prompt and enter an empty build directory.

Your Python interpreter, Meson executable script and Ninja (if used)
need to be in your PATH.

Any dependent libraries that are being used should have their headers
found in paths specified by %INCLUDE% and their .lib files in the
paths specified by %LIB%.

In the empty build directory, run the following:

meson <path_to_libpsl_git_checkout> --buildtype=... --prefix=<some_prefix> [--backend=vs[2010|2015|2017|2019]]

Please see the Meson documentation for the values accepted by
--buildtype.  --backend=vsXXXX generates the corresponding versions
of the Visual Studio solution files to build libpsl, which
will elimnate the need to have the Ninja build tool installed.

When the Meson configuration completes, run 'ninja' or open the
generated solution files with Visual Studio and build the projects
to carry out the build.  Run 'ninja test' or the test project to
test the build and run 'ninja install' or 'ninja install' to
install the build results.

If building with Visual Studio 2008, run the following after running
'ninja install' in your builddir:

for /r %f in (*.dll.manifest) do if exist $(prefix)\bin\%~nf mt /manifest %f /outputresource:$(prefix)\bin\%~nf;2

for /r %f in (*.exe.manifest) do if exist $(prefix)\bin\%~nf mt /manifest %f /outputresource:$(prefix)\bin\%~nf;1

So that the application manifests get properly embedded.