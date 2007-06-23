/* rndlinux.c  -  raw random number for OSes with /dev/random
 * Copyright (C) 1998, 2001, 2002, 2003  Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_GETTIMEOFDAY
# include <sys/times.h>
#endif
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "types.h"
#include "g10lib.h"
#include "rand-internal.h"

static int open_device( const char *name, int minor );
int _gcry_rndlinux_gather_random (void (*add)(const void*, size_t, int),
                                  int requester,
                                  size_t length, int level );

static int
set_cloexec_flag (int fd)
{
  int oldflags;

  oldflags= fcntl (fd, F_GETFD, 0);
  if (oldflags < 0)
    return oldflags;
  oldflags |= FD_CLOEXEC;
  return fcntl (fd, F_SETFD, oldflags);
}


/*
 * Used to open the /dev/random devices (Linux, xBSD, Solaris (if it exists)).
 */
static int
open_device( const char *name, int minor )
{
  int fd;

  fd = open( name, O_RDONLY );
  if( fd == -1 )
    log_fatal ("can't open %s: %s\n", name, strerror(errno) );

  if (set_cloexec_flag (fd))
    log_error ("error setting FD_CLOEXEC on fd %d: %s\n",
               fd, strerror (errno));

  /* We used to do the follwing check, however it turned out that this
     is not portable since more OSes provide a random device which is
     sometimes implemented as anoteher device type. 
     
     struct stat sb;

     if( fstat( fd, &sb ) )
        log_fatal("stat() off %s failed: %s\n", name, strerror(errno) );
     if( (!S_ISCHR(sb.st_mode)) && (!S_ISFIFO(sb.st_mode)) )
        log_fatal("invalid random device!\n" );
  */
  return fd;
}


int
_gcry_rndlinux_gather_random (void (*add)(const void*, size_t, int),
                              int requester,
                              size_t length, int level )
{
  static int fd_urandom = -1;
  static int fd_random = -1;
  int fd;
  int n;
  int warn=0;
  byte buffer[768];

  if( level >= 2 )
    {
      if( fd_random == -1 )
        fd_random = open_device( NAME_OF_DEV_RANDOM, 8 );
      fd = fd_random;
    }
  else
    {
      if( fd_urandom == -1 )
        fd_urandom = open_device( NAME_OF_DEV_URANDOM, 9 );
      fd = fd_urandom;
    }

  while (length)
    {
      fd_set rfds;
      struct timeval tv;
      int rc;
      
      FD_ZERO(&rfds);
      FD_SET(fd, &rfds);
      tv.tv_sec = 3;
      tv.tv_usec = 0;
      if( !(rc=select(fd+1, &rfds, NULL, NULL, &tv)) )
        {
          if( !warn )
            {
              _gcry_random_progress ("need_entropy", 'X', 0, (int)length);
	      warn = 1;
	    }
	  continue;
	}
	else if( rc == -1 )
          {
	    log_error ("select() error: %s\n", strerror(errno));
	    continue;
          }

	do 
          {
	    int nbytes = length < sizeof(buffer)? length : sizeof(buffer);
	    n = read(fd, buffer, nbytes );
	    if( n >= 0 && n > nbytes ) 
              {
		log_error("bogus read from random device (n=%d)\n", n );
		n = nbytes;
              }
          } 
        while( n == -1 && errno == EINTR );
	if( n == -1 )
          log_fatal("read error on random device: %s\n", strerror(errno));
	(*add)( buffer, n, requester );
	length -= n;
    }
  memset(buffer, 0, sizeof(buffer) );

  return 0; /* success */
}
