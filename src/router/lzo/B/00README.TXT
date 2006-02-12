Simple make drivers for DOS, Windows, OS/2 and other systems
============================================================

This directory contains a bunch of simple make drivers - I've tried
to make them as foolproof as possible.

To build LZO for your system type `b\OS\COMPILER' in the base directory,
e.g. `b\win32\mc120.bat' will build the Win32 Microsoft C v12.0 (aka
Visual C++ 6) version.

After building do a basic test by running `ltest -mlzo COPYING'.
util\check.sh is an example of a more thorough test driver.

Please send me your additional/improved versions.


Overview:
---------

b\dos16\bc40.bat        Borland C 4.0
b\dos16\dmc.bat         Digital Mars C/C++ 8.28
b\dos16\mc70.bat        Microsoft C 7.0
b\dos16\mc80.bat        Microsoft C 8.0        Visual C 1.5
b\dos16\wc105.bat       Watcom C 10.5

b\dos32\dj1.bat         djgpp v1 + gcc 2.6.3                  (1)
b\dos32\dj2.bat         djgpp v2 + gcc                        (1)
b\dos32\dmc.bat         Digital Mars C/C++ 8.28
b\dos32\emx.bat         emx + gcc                             (1)
b\dos32\wc105.bat       Watcom C 10.5                         (1)

b\os2\emx_aout.cmd      emx A.OUT + gcc                       (1) (3)
b\os2\wc105.cmd         Watcom C 10.5                         (1) (3)

b\palmos\gcc_lib.sh     gcc 2.7.2 cross-compiler

b\tos\gcc258.g          gcc 2.5.8
b\tos\pc.prj            Pure C 1.0 (Sep 20 1991)

b\win16\wc105.bat       Watcom C 10.5
b\win16\wc105dll.bat    Watcom C 10.5 DLL

b\win32\bc55.bat        Borland C 5.5.1
b\win32\dmc.bat         Digital Mars C/C++ 8.28               (1)
b\win32\cygwin.bat      Cygnus Win32 + gcc                    (1)
b\win32\mc120.bat       Microsoft C 12.0       Visual C 6.0   (2)
b\win32\mc120dll.bat    Microsoft C 12.0 DLL
b\win32\mingw.bat       Mingw32 + gcc                         (1)
b\win32\wc105.bat       Watcom C 10.5                         (1)
b\win32\wc105dll.bat    Watcom C 10.5 DLL


Notes:
  (1) includes support for i386 assembler versions
  (2) you need MASM (or another assembler) for the assembler versions
  (3) not tested as of version 1.08



Useful tools
============

GNU make, GNU fileutils for DOS / Windows 3.1 / Windows 95/98/ME:
   ftp://ftp.simtel.net/pub/simtelnet/gnu/djgpp/v2gnu/

GNU make, GNU fileutils for Windows NT/2000
   ftp://ftp.cygnus.com

GNU make, GNU fileutils for OS2
   ftp://hobbes.nmsu.edu/pub/os2/

GNU make, GNU fileutils, gulam, MiNT for TOS (Atari ST)
   http://www.freemint.de
   ftp://archive.umich.edu

nasm assembler
   ftp://ftp.simtel.net/pub/simtelnet/asmutl/
   ftp://sunsite.unc.edu/pub/Linux/devel/lang/assemblers/

