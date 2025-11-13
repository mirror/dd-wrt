/* Retrieve information about a FILE stream.
   Copyright (C) 2007-2022 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "fwriting.h"

#include "stdio-impl.h"

/* This file is not used on systems that have the __fwriting function,
   namely glibc >= 2.2, Solaris >= 7, UnixWare >= 7.1.4.MP4, Cygwin >= 1.7.34,
   Android API >= 29, musl libc.  */

bool
fwriting (FILE *fp)
{
  /* Most systems provide FILE as a struct and the necessary bitmask in
     <stdio.h>, because they need it for implementing getc() and putc() as
     fast macros.  */
#if defined _IO_EOF_SEEN || defined _IO_ftrylockfile || __GNU_LIBRARY__ == 1
  /* GNU libc, BeOS, Haiku, Linux libc5 */
  return (fp->_flags & (_IO_NO_READS | _IO_CURRENTLY_PUTTING)) != 0;
#elif defined __sferror || defined __DragonFly__ || defined __ANDROID__
  /* FreeBSD, NetBSD, OpenBSD, DragonFly, Mac OS X, Cygwin < 1.7.34, Minix 3, Android */
  return (fp_->_flags & __SWR) != 0;
#elif defined __EMX__               /* emx+gcc */
  return (fp->_flags & _IOWRT) != 0;
#elif defined __minix               /* Minix */
  return (fp->_flags & _IOWRITING) != 0;
#elif defined _IOERR                /* AIX, HP-UX, IRIX, OSF/1, Solaris, OpenServer, UnixWare, mingw, MSVC, NonStop Kernel, OpenVMS */
  return (fp_->_flag & _IOWRT) != 0;
#elif defined __UCLIBC__            /* uClibc */
  return (fp->__modeflags & __FLAG_WRITING) != 0;
#elif defined __QNX__               /* QNX */
  return ((fp->_Mode & 0x1 /* _MOPENR */) == 0
          || (fp->_Mode & 0x2000 /* _MWRITE */) != 0);
#elif defined __MINT__              /* Atari FreeMiNT */
  if (!fp->__mode.__read)
    return 1;
  if (!fp->__mode.__write)
    return 0;
# ifdef _IO_CURRENTLY_PUTTING /* Flag added on 2009-02-28 */
  return (fp->__flags & _IO_CURRENTLY_PUTTING) != 0;
# else
  return (fp->__buffer < fp->__put_limit /*|| fp->__bufp == fp->__get_limit ??*/);
# endif
#elif defined EPLAN9                /* Plan9 */
  if (fp->state == 0 /* CLOSED */ || fp->state == 3 /* RD */)
    return 0;
  return (fp->state == 4 /* WR */ && (fp->bufl == 0 || fp->wp < fp->rp));
#else
# error "Please port gnulib fwriting.c to your platform!"
#endif
}
