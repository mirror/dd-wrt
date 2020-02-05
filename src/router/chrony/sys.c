/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
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

  This file contains all the conditionally compiled bits that pull
  in the various operating-system specific modules
  */

#include "config.h"

#include "sysincl.h"

#include "sys.h"
#include "sys_null.h"
#include "logging.h"

#if defined(LINUX)
#include "sys_linux.h"
#include "sys_posix.h"
#elif defined(SOLARIS)
#include "sys_solaris.h"
#include "sys_posix.h"
#elif defined(NETBSD) || defined(FREEBSD)
#include "sys_netbsd.h"
#include "sys_posix.h"
#elif defined(MACOSX)
#include "sys_macosx.h"
#endif

/* ================================================== */

static int null_driver;

/* ================================================== */

void
SYS_Initialise(int clock_control)
{
  null_driver = !clock_control;
  if (null_driver) {
    SYS_Null_Initialise();
    return;
  }
#if defined(LINUX)
  SYS_Linux_Initialise();
#elif defined(SOLARIS)
  SYS_Solaris_Initialise();
#elif defined(NETBSD) || defined(FREEBSD)
  SYS_NetBSD_Initialise();
#elif defined(MACOSX)
  SYS_MacOSX_Initialise();
#else
#error Unknown system
#endif
}

/* ================================================== */

void
SYS_Finalise(void)
{
  if (null_driver) {
    SYS_Null_Finalise();
    return;
  }
#if defined(LINUX)
  SYS_Linux_Finalise();
#elif defined(SOLARIS)
  SYS_Solaris_Finalise();
#elif defined(NETBSD) || defined(FREEBSD)
  SYS_NetBSD_Finalise();
#elif defined(MACOSX)
  SYS_MacOSX_Finalise();
#else
#error Unknown system
#endif
}

/* ================================================== */

void SYS_DropRoot(uid_t uid, gid_t gid)
{
#if defined(LINUX) && defined (FEAT_PRIVDROP)
  SYS_Linux_DropRoot(uid, gid, !null_driver);
#elif defined(SOLARIS) && defined(FEAT_PRIVDROP)
  SYS_Solaris_DropRoot(uid, gid);
#elif (defined(NETBSD) || defined(FREEBSD)) && defined(FEAT_PRIVDROP)
  SYS_NetBSD_DropRoot(uid, gid);
#elif defined(MACOSX) && defined(FEAT_PRIVDROP)
  SYS_MacOSX_DropRoot(uid, gid);
#else
  LOG_FATAL("dropping root privileges not supported");
#endif
}

/* ================================================== */

void SYS_EnableSystemCallFilter(int level)
{
#if defined(LINUX) && defined(FEAT_SCFILTER)
  SYS_Linux_EnableSystemCallFilter(level);
#else
  LOG_FATAL("system call filter not supported");
#endif
}

/* ================================================== */

void SYS_SetScheduler(int SchedPriority)
{
#if defined(MACOSX)
  SYS_MacOSX_SetScheduler(SchedPriority);
#elif defined(HAVE_PTHREAD_SETSCHEDPARAM)
  SYS_Posix_SetScheduler(SchedPriority);
#else
  LOG_FATAL("scheduler priority setting not supported");
#endif
}

/* ================================================== */

void SYS_LockMemory(void)
{
#if defined(HAVE_MLOCKALL)
  SYS_Posix_MemLockAll();
#else
  LOG_FATAL("memory locking not supported");
#endif
}

/* ================================================== */
