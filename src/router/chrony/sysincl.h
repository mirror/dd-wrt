/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  This file includes all system header files that the software
  requires.  This allows us to isolate system dependencies to this file
  alone.
  */

#ifndef GOT_SYSINCL_H
#define GOT_SYSINCL_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <glob.h>
#include <grp.h>
#include <inttypes.h>
#include <math.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#if defined(LINUX) || defined(FREEBSD) || defined(NETBSD) || defined(SOLARIS) || defined(HAVE_MACOS_SYS_TIMEX)
#include <sys/timex.h>
#endif

#ifdef FEAT_IPV6
/* For inet_ntop() */
#include <arpa/inet.h>
#endif

#ifdef HAVE_GETRANDOM
#include <sys/random.h>
#endif

#endif /* GOT_SYSINCL_H */
