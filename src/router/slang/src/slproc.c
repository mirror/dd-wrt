/* Process specific system calls */
/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE
#endif
#ifndef _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED 1
#endif

#ifndef __EXTENSIONS__
# define __EXTENSIONS__
#endif

#include "slinclud.h"

#ifdef HAVE_IO_H
# include <io.h>		       /* for chmod */
#endif

#ifdef HAVE_PROCESS_H
# include <process.h>			/* for getpid */
#endif

#if defined(__BORLANDC__)
# include <dos.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#include <errno.h>

#include "slang.h"
#include "_slang.h"

#ifdef HAVE_KILL
static int kill_cmd (int *pid, int *sig)
{
   int ret;

   if (-1 == (ret = kill ((pid_t) *pid, *sig)))
     _pSLerrno_errno = errno;
   return ret;
}
#endif

#ifdef HAVE_KILLPG
static int killpg_cmd (int *pgrp, int *sig)
{
   int ret;

   if (-1 == (ret = killpg ((pid_t) *pgrp, *sig)))
     _pSLerrno_errno = errno;
   return ret;
}
#endif

static int getpid_cmd (void)
{
   return getpid ();
}

#ifdef HAVE_GETPPID
static int getppid_cmd (void)
{
   return getppid ();
}
#endif

#ifdef HAVE_GETGID
static int getgid_cmd (void)
{
   return getgid ();
}
#endif

#ifdef HAVE_GETEGID
static int getegid_cmd (void)
{
   return getegid ();
}
#endif

#ifdef HAVE_GETEUID
static int geteuid_cmd (void)
{
   return geteuid ();
}
#endif

#ifdef HAVE_GETUID
static int getuid_cmd (void)
{
   return getuid ();
}
#endif

#ifdef HAVE_SETGID
static int setgid_cmd (int *gid)
{
   if (0 == setgid (*gid))
     return 0;
   _pSLerrno_errno = errno;
   return -1;
}
#endif

#ifdef HAVE_SETPGID
static int setpgid_cmd (int *pid, int *pgid)
{
   if (0 == setpgid (*pid, *pgid))
     return 0;
   _pSLerrno_errno = errno;
   return -1;
}
#endif

#ifdef HAVE_GETPGID
static int getpgid_cmd (int *pid)
{
   int pgid = getpgid (*pid);
   if (-1 != pgid)
     return pgid;
   _pSLerrno_errno = errno;
   return -1;
}
#endif

#ifdef HAVE_GETPGRP
static int getpgrp_cmd (void)
{
   int pgid = getpgrp ();
   if (-1 != pgid)
     return pgid;
   _pSLerrno_errno = errno;
   return -1;
}
#endif

#ifdef HAVE_SETPGRP
#if 0
static int setpgrp_cmd (void)
{
   if (0 == setpgrp ())
     return 0;
   _pSLerrno_errno = errno;
   return -1;
}
#endif
#endif

#ifdef HAVE_SETUID
static int setuid_cmd (int *uid)
{
   if (0 == setuid (*uid))
     return 0;
   _pSLerrno_errno = errno;
   return -1;
}
#endif

#ifdef HAVE_SETSID
static int setsid_cmd (void)
{
   pid_t pid = setsid ();

   if (pid == (pid_t)-1)
     _pSLerrno_errno = errno;
   return pid;
}
#endif

#ifdef HAVE_GETSID
static int getsid_cmd (void)
{
   int ipid = 0;
   pid_t pid;

   if ((SLang_Num_Function_Args == 1)
       && (-1 == SLang_pop_int (&ipid)))
     return -1;

   pid = getsid (ipid);

   if (pid == (pid_t)-1)
     _pSLerrno_errno = errno;
   return pid;
}
#endif

#ifdef HAVE_GETPRIORITY
static void getpriority_intrin (int *which, int *who)
{
   int ret;
   errno = 0;
   ret = getpriority (*which, *who);
   if ((ret == -1) && (errno != 0))
     {
	_pSLerrno_errno = errno;
	(void) SLang_push_null ();
	return;
     }
   (void) SLang_push_int (ret);
}
#endif

#ifdef HAVE_SETPRIORITY
static int setpriority_intrin (int *which, int *who, int *prio)
{
   int ret;
   if (-1 == (ret = setpriority (*which, *who, *prio)))
     _pSLerrno_errno = errno;
   return ret;
}
#endif

static SLang_Intrin_Fun_Type Process_Name_Table[] =
{
   MAKE_INTRINSIC_0("getpid", getpid_cmd, SLANG_INT_TYPE),

#ifdef HAVE_SETSID
   MAKE_INTRINSIC_0("setsid", setsid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETSID
   MAKE_INTRINSIC_0("getsid", getsid_cmd, SLANG_INT_TYPE),
#endif

#ifdef HAVE_GETPPID
   MAKE_INTRINSIC_0("getppid", getppid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETGID
   MAKE_INTRINSIC_0("getgid", getgid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETEGID
   MAKE_INTRINSIC_0("getegid", getegid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETEUID
   MAKE_INTRINSIC_0("geteuid", geteuid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETUID
   MAKE_INTRINSIC_0("getuid", getuid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETGID
   MAKE_INTRINSIC_0("getgid", getgid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_SETGID
   MAKE_INTRINSIC_I("setgid", setgid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETPGID
   MAKE_INTRINSIC_I("getpgid", getpgid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_SETPGID
   MAKE_INTRINSIC_II("setpgid", setpgid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETPGRP
   MAKE_INTRINSIC_0("getpgrp", getpgrp_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_SETPGRP
   /* MAKE_INTRINSIC_0("setpgrp", setpgrp_cmd, SLANG_INT_TYPE), */
#endif
#ifdef HAVE_SETUID
   MAKE_INTRINSIC_I("setuid", setuid_cmd, SLANG_INT_TYPE),
#endif

#ifdef HAVE_KILL
   MAKE_INTRINSIC_II("kill", kill_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_KILLPG
   MAKE_INTRINSIC_II("killpg", killpg_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_SETPRIORITY
   MAKE_INTRINSIC_III("setpriority", setpriority_intrin, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETPRIORITY
   MAKE_INTRINSIC_II("getpriority", getpriority_intrin, SLANG_VOID_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_IConstant_Type Const_Table [] =
{
#ifdef PRIO_PROCESS
   MAKE_ICONSTANT("PRIO_PROCESS", PRIO_PROCESS),
#endif
#ifdef PRIO_PGRP
   MAKE_ICONSTANT("PRIO_PGRP", PRIO_PGRP),
#endif
#ifdef PRIO_USER
   MAKE_ICONSTANT("PRIO_USER", PRIO_USER),
#endif
   SLANG_END_ICONST_TABLE
};

int SLang_init_posix_process (void)
{
   if ((-1 == SLadd_intrin_fun_table (Process_Name_Table, "__POSIX_PROCESS__"))
       || (-1 == SLadd_iconstant_table (Const_Table, NULL))
       || (-1 == _pSLerrno_init ()))
     return -1;

   return 0;
}
