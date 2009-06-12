/* lockfile - Handle locking and unlocking of stream.
   Copyright (C) 1996, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <stdio.h>
#include <pthread.h>

void
__flockfile (FILE *stream)
{
  pthread_mutex_lock(&stream->lock);
}
weak_alias (__flockfile, flockfile);


void
__funlockfile (FILE *stream)
{
  pthread_mutex_unlock(&stream->lock);
}
weak_alias (__funlockfile, funlockfile);


int
__ftrylockfile (FILE *stream)
{
  return pthread_mutex_trylock(&stream->lock);
}
weak_alias (__ftrylockfile, ftrylockfile);


void
__fresetlockfiles (void)
{
  FILE *fp;
  pthread_mutexattr_t attr;

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

  for (fp = _stdio_openlist; fp != NULL; fp = fp->nextopen)
    pthread_mutex_init(&fp->lock, &attr);

  pthread_mutexattr_destroy(&attr);
}
