Notes for m68k-palmos-coff-gcc 2.7.2.2-kgpd-071097 cross-compiler under Win32.

I had to patch the following include files (see below)
1) both <sys/types.h> and <stddef.h> typedef size_t
2) <limits.h> is broken as gcc seems to imply the `-mshort' switch

If your compiler doesn't find <stdlib.h> then copy the file
m68k-palmos-coff/include/PalmOS/System/Unix/unix_stdlib.h


Cross-compilation using m68k-palmos-coff.tools040a2.Linux-ELF.tar.gz
didn't work for me because of missing include files and libraries.


*** lib/gcc-lib/m68k-palmos-coff/2.7.2.2-kgpd-071097/include/sys/types.org	Thu Jul 10 08:33:21 1997
--- lib/gcc-lib/m68k-palmos-coff/2.7.2.2-kgpd-071097/include/sys/types.h	Mon Dec 22 23:46:35 1997
***************
*** 45,49 ****
--- 45,51 ----
  typedef unsigned int tcflag_t;

+ #if 0
  typedef unsigned long size_t;
+ #endif

  #endif


*** lib/gcc-lib/m68k-palmos-coff/2.7.2.2-kgpd-071097/include/limits.org	Thu Jul 10 08:25:01 1997
--- lib/gcc-lib/m68k-palmos-coff/2.7.2.2-kgpd-071097/include/limits.h	Tue Dec 23 01:21:29 1997
***************
*** 40,44 ****
  /* Minimum and maximum values a `signed short int' can hold.  */
  #undef SHRT_MIN
! #define SHRT_MIN (-32768)
  #undef SHRT_MAX
  #define SHRT_MAX 32767
--- 40,44 ----
  /* Minimum and maximum values a `signed short int' can hold.  */
  #undef SHRT_MIN
! #define SHRT_MIN (-32767-1)
  #undef SHRT_MAX
  #define SHRT_MAX 32767
***************
*** 46,54 ****
  /* Maximum value an `unsigned short int' can hold.  (Minimum is 0).  */
  #undef USHRT_MAX
! #define USHRT_MAX 65535

  /* Minimum and maximum values a `signed int' can hold.  */
  #ifndef __INT_MAX__
! #define __INT_MAX__ 2147483647
  #endif
  #undef INT_MIN
--- 46,54 ----
  /* Maximum value an `unsigned short int' can hold.  (Minimum is 0).  */
  #undef USHRT_MAX
! #define USHRT_MAX 65535U

  /* Minimum and maximum values a `signed int' can hold.  */
  #ifndef __INT_MAX__
! #define __INT_MAX__ 32767
  #endif
  #undef INT_MIN
