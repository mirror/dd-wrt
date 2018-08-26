/* -*- mode: C; mode: fold; -*- */
/*
Copyright (C) 2009-2011 John E. Davis

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

#include "config.h"

#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <slang.h>

SLANG_MODULE(sysconf);

typedef struct
{
   const char *name;
   const unsigned int namelen;
   const int iname;
}
Name_Map_Type;

static const Name_Map_Type *lookup_name (const Name_Map_Type *map, char *name)
{
   unsigned int len = strlen (name);
   while (map->namelen != 0)
     {
	if ((map->namelen == len)
	    && (0 == strcmp (name, map->name)))
	  return map;

	map++;
     }
   return NULL;
}

static const Name_Map_Type SC_Name_Map_Table[] = /*{{{*/
{
#ifdef _SC_ARG_MAX
   {"_SC_ARG_MAX",	11,	_SC_ARG_MAX},
#endif
#ifdef _SC_CHILD_MAX
   {"_SC_CHILD_MAX",	13,	_SC_CHILD_MAX},
#endif
#ifdef _SC_CLK_TCK
   {"_SC_CLK_TCK",	11,	_SC_CLK_TCK},
#endif
#ifdef _SC_NGROUPS_MAX
   {"_SC_NGROUPS_MAX",	15,	_SC_NGROUPS_MAX},
#endif
#ifdef _SC_OPEN_MAX
   {"_SC_OPEN_MAX",	12,	_SC_OPEN_MAX},
#endif
#ifdef _SC_STREAM_MAX
   {"_SC_STREAM_MAX",	14,	_SC_STREAM_MAX},
#endif
#ifdef _SC_TZNAME_MAX
   {"_SC_TZNAME_MAX",	14,	_SC_TZNAME_MAX},
#endif
#ifdef _SC_JOB_CONTROL
   {"_SC_JOB_CONTROL",	15,	_SC_JOB_CONTROL},
#endif
#ifdef _SC_SAVED_IDS
   {"_SC_SAVED_IDS",	13,	_SC_SAVED_IDS},
#endif
#ifdef _SC_REALTIME_SIGNALS
   {"_SC_REALTIME_SIGNALS",	20,	_SC_REALTIME_SIGNALS},
#endif
#ifdef _SC_PRIORITY_SCHEDULING
   {"_SC_PRIORITY_SCHEDULING",	23,	_SC_PRIORITY_SCHEDULING},
#endif
#ifdef _SC_TIMERS
   {"_SC_TIMERS",	10,	_SC_TIMERS},
#endif
#ifdef _SC_ASYNCHRONOUS_IO
   {"_SC_ASYNCHRONOUS_IO",	19,	_SC_ASYNCHRONOUS_IO},
#endif
#ifdef _SC_PRIORITIZED_IO
   {"_SC_PRIORITIZED_IO",	18,	_SC_PRIORITIZED_IO},
#endif
#ifdef _SC_SYNCHRONIZED_IO
   {"_SC_SYNCHRONIZED_IO",	19,	_SC_SYNCHRONIZED_IO},
#endif
#ifdef _SC_FSYNC
   {"_SC_FSYNC",	9,	_SC_FSYNC},
#endif
#ifdef _SC_MAPPED_FILES
   {"_SC_MAPPED_FILES",	16,	_SC_MAPPED_FILES},
#endif
#ifdef _SC_MEMLOCK
   {"_SC_MEMLOCK",	11,	_SC_MEMLOCK},
#endif
#ifdef _SC_MEMLOCK_RANGE
   {"_SC_MEMLOCK_RANGE",	17,	_SC_MEMLOCK_RANGE},
#endif
#ifdef _SC_MEMORY_PROTECTION
   {"_SC_MEMORY_PROTECTION",	21,	_SC_MEMORY_PROTECTION},
#endif
#ifdef _SC_MESSAGE_PASSING
   {"_SC_MESSAGE_PASSING",	19,	_SC_MESSAGE_PASSING},
#endif
#ifdef _SC_SEMAPHORES
   {"_SC_SEMAPHORES",	14,	_SC_SEMAPHORES},
#endif
#ifdef _SC_SHARED_MEMORY_OBJECTS
   {"_SC_SHARED_MEMORY_OBJECTS",	25,	_SC_SHARED_MEMORY_OBJECTS},
#endif
#ifdef _SC_AIO_LISTIO_MAX
   {"_SC_AIO_LISTIO_MAX",	18,	_SC_AIO_LISTIO_MAX},
#endif
#ifdef _SC_AIO_MAX
   {"_SC_AIO_MAX",	11,	_SC_AIO_MAX},
#endif
#ifdef _SC_AIO_PRIO_DELTA_MAX
   {"_SC_AIO_PRIO_DELTA_MAX",	22,	_SC_AIO_PRIO_DELTA_MAX},
#endif
#ifdef _SC_DELAYTIMER_MAX
   {"_SC_DELAYTIMER_MAX",	18,	_SC_DELAYTIMER_MAX},
#endif
#ifdef _SC_MQ_OPEN_MAX
   {"_SC_MQ_OPEN_MAX",	15,	_SC_MQ_OPEN_MAX},
#endif
#ifdef _SC_MQ_PRIO_MAX
   {"_SC_MQ_PRIO_MAX",	15,	_SC_MQ_PRIO_MAX},
#endif
#ifdef _SC_VERSION
   {"_SC_VERSION",	11,	_SC_VERSION},
#endif
#ifdef _SC_PAGESIZE
   {"_SC_PAGESIZE",	12,	_SC_PAGESIZE},
#endif
#ifdef _SC_PAGE_SIZE
   {"_SC_PAGE_SIZE",	13,	_SC_PAGE_SIZE},
#endif
#ifdef _SC_RTSIG_MAX
   {"_SC_RTSIG_MAX",	13,	_SC_RTSIG_MAX},
#endif
#ifdef _SC_SEM_NSEMS_MAX
   {"_SC_SEM_NSEMS_MAX",	17,	_SC_SEM_NSEMS_MAX},
#endif
#ifdef _SC_SEM_VALUE_MAX
   {"_SC_SEM_VALUE_MAX",	17,	_SC_SEM_VALUE_MAX},
#endif
#ifdef _SC_SIGQUEUE_MAX
   {"_SC_SIGQUEUE_MAX",	16,	_SC_SIGQUEUE_MAX},
#endif
#ifdef _SC_TIMER_MAX
   {"_SC_TIMER_MAX",	13,	_SC_TIMER_MAX},
#endif
#ifdef _SC_BC_BASE_MAX
   {"_SC_BC_BASE_MAX",	15,	_SC_BC_BASE_MAX},
#endif
#ifdef _SC_BC_DIM_MAX
   {"_SC_BC_DIM_MAX",	14,	_SC_BC_DIM_MAX},
#endif
#ifdef _SC_BC_SCALE_MAX
   {"_SC_BC_SCALE_MAX",	16,	_SC_BC_SCALE_MAX},
#endif
#ifdef _SC_BC_STRING_MAX
   {"_SC_BC_STRING_MAX",	17,	_SC_BC_STRING_MAX},
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
   {"_SC_COLL_WEIGHTS_MAX",	20,	_SC_COLL_WEIGHTS_MAX},
#endif
#ifdef _SC_EQUIV_CLASS_MAX
   {"_SC_EQUIV_CLASS_MAX",	19,	_SC_EQUIV_CLASS_MAX},
#endif
#ifdef _SC_EXPR_NEST_MAX
   {"_SC_EXPR_NEST_MAX",	17,	_SC_EXPR_NEST_MAX},
#endif
#ifdef _SC_LINE_MAX
   {"_SC_LINE_MAX",	12,	_SC_LINE_MAX},
#endif
#ifdef _SC_RE_DUP_MAX
   {"_SC_RE_DUP_MAX",	14,	_SC_RE_DUP_MAX},
#endif
#ifdef _SC_CHARCLASS_NAME_MAX
   {"_SC_CHARCLASS_NAME_MAX",	22,	_SC_CHARCLASS_NAME_MAX},
#endif
#ifdef _SC_2_VERSION
   {"_SC_2_VERSION",	13,	_SC_2_VERSION},
#endif
#ifdef _SC_2_C_BIND
   {"_SC_2_C_BIND",	12,	_SC_2_C_BIND},
#endif
#ifdef _SC_2_C_DEV
   {"_SC_2_C_DEV",	11,	_SC_2_C_DEV},
#endif
#ifdef _SC_2_FORT_DEV
   {"_SC_2_FORT_DEV",	14,	_SC_2_FORT_DEV},
#endif
#ifdef _SC_2_FORT_RUN
   {"_SC_2_FORT_RUN",	14,	_SC_2_FORT_RUN},
#endif
#ifdef _SC_2_SW_DEV
   {"_SC_2_SW_DEV",	12,	_SC_2_SW_DEV},
#endif
#ifdef _SC_2_LOCALEDEF
   {"_SC_2_LOCALEDEF",	15,	_SC_2_LOCALEDEF},
#endif
#ifdef _SC_PII
   {"_SC_PII",	7,	_SC_PII},
#endif
#ifdef _SC_PII_XTI
   {"_SC_PII_XTI",	11,	_SC_PII_XTI},
#endif
#ifdef _SC_PII_SOCKET
   {"_SC_PII_SOCKET",	14,	_SC_PII_SOCKET},
#endif
#ifdef _SC_PII_INTERNET
   {"_SC_PII_INTERNET",	16,	_SC_PII_INTERNET},
#endif
#ifdef _SC_PII_OSI
   {"_SC_PII_OSI",	11,	_SC_PII_OSI},
#endif
#ifdef _SC_POLL
   {"_SC_POLL",	8,	_SC_POLL},
#endif
#ifdef _SC_SELECT
   {"_SC_SELECT",	10,	_SC_SELECT},
#endif
#ifdef _SC_UIO_MAXIOV
   {"_SC_UIO_MAXIOV",	14,	_SC_UIO_MAXIOV},
#endif
#ifdef _SC_IOV_MAX
   {"_SC_IOV_MAX",	11,	_SC_IOV_MAX},
#endif
#ifdef _SC_PII_INTERNET_STREAM
   {"_SC_PII_INTERNET_STREAM",	23,	_SC_PII_INTERNET_STREAM},
#endif
#ifdef _SC_PII_INTERNET_DGRAM
   {"_SC_PII_INTERNET_DGRAM",	22,	_SC_PII_INTERNET_DGRAM},
#endif
#ifdef _SC_PII_OSI_COTS
   {"_SC_PII_OSI_COTS",	16,	_SC_PII_OSI_COTS},
#endif
#ifdef _SC_PII_OSI_CLTS
   {"_SC_PII_OSI_CLTS",	16,	_SC_PII_OSI_CLTS},
#endif
#ifdef _SC_PII_OSI_M
   {"_SC_PII_OSI_M",	13,	_SC_PII_OSI_M},
#endif
#ifdef _SC_T_IOV_MAX
   {"_SC_T_IOV_MAX",	13,	_SC_T_IOV_MAX},
#endif
#ifdef _SC_THREADS
   {"_SC_THREADS",	11,	_SC_THREADS},
#endif
#ifdef _SC_THREAD_SAFE_FUNCTIONS
   {"_SC_THREAD_SAFE_FUNCTIONS",	25,	_SC_THREAD_SAFE_FUNCTIONS},
#endif
#ifdef _SC_GETGR_R_SIZE_MAX
   {"_SC_GETGR_R_SIZE_MAX",	20,	_SC_GETGR_R_SIZE_MAX},
#endif
#ifdef _SC_GETPW_R_SIZE_MAX
   {"_SC_GETPW_R_SIZE_MAX",	20,	_SC_GETPW_R_SIZE_MAX},
#endif
#ifdef _SC_LOGIN_NAME_MAX
   {"_SC_LOGIN_NAME_MAX",	18,	_SC_LOGIN_NAME_MAX},
#endif
#ifdef _SC_TTY_NAME_MAX
   {"_SC_TTY_NAME_MAX",	16,	_SC_TTY_NAME_MAX},
#endif
#ifdef _SC_THREAD_DESTRUCTOR_ITERATIONS
   {"_SC_THREAD_DESTRUCTOR_ITERATIONS",	32,	_SC_THREAD_DESTRUCTOR_ITERATIONS},
#endif
#ifdef _SC_THREAD_KEYS_MAX
   {"_SC_THREAD_KEYS_MAX",	19,	_SC_THREAD_KEYS_MAX},
#endif
#ifdef _SC_THREAD_STACK_MIN
   {"_SC_THREAD_STACK_MIN",	20,	_SC_THREAD_STACK_MIN},
#endif
#ifdef _SC_THREAD_THREADS_MAX
   {"_SC_THREAD_THREADS_MAX",	22,	_SC_THREAD_THREADS_MAX},
#endif
#ifdef _SC_THREAD_ATTR_STACKADDR
   {"_SC_THREAD_ATTR_STACKADDR",	25,	_SC_THREAD_ATTR_STACKADDR},
#endif
#ifdef _SC_THREAD_ATTR_STACKSIZE
   {"_SC_THREAD_ATTR_STACKSIZE",	25,	_SC_THREAD_ATTR_STACKSIZE},
#endif
#ifdef _SC_THREAD_PRIORITY_SCHEDULING
   {"_SC_THREAD_PRIORITY_SCHEDULING",	30,	_SC_THREAD_PRIORITY_SCHEDULING},
#endif
#ifdef _SC_THREAD_PRIO_INHERIT
   {"_SC_THREAD_PRIO_INHERIT",	23,	_SC_THREAD_PRIO_INHERIT},
#endif
#ifdef _SC_THREAD_PRIO_PROTECT
   {"_SC_THREAD_PRIO_PROTECT",	23,	_SC_THREAD_PRIO_PROTECT},
#endif
#ifdef _SC_THREAD_PROCESS_SHARED
   {"_SC_THREAD_PROCESS_SHARED",	25,	_SC_THREAD_PROCESS_SHARED},
#endif
#ifdef _SC_NPROCESSORS_CONF
   {"_SC_NPROCESSORS_CONF",	20,	_SC_NPROCESSORS_CONF},
#endif
#ifdef _SC_NPROCESSORS_ONLN
   {"_SC_NPROCESSORS_ONLN",	20,	_SC_NPROCESSORS_ONLN},
#endif
#ifdef _SC_PHYS_PAGES
   {"_SC_PHYS_PAGES",	14,	_SC_PHYS_PAGES},
#endif
#ifdef _SC_AVPHYS_PAGES
   {"_SC_AVPHYS_PAGES",	16,	_SC_AVPHYS_PAGES},
#endif
#ifdef _SC_ATEXIT_MAX
   {"_SC_ATEXIT_MAX",	14,	_SC_ATEXIT_MAX},
#endif
#ifdef _SC_PASS_MAX
   {"_SC_PASS_MAX",	12,	_SC_PASS_MAX},
#endif
#ifdef _SC_XOPEN_VERSION
   {"_SC_XOPEN_VERSION",	17,	_SC_XOPEN_VERSION},
#endif
#ifdef _SC_XOPEN_XCU_VERSION
   {"_SC_XOPEN_XCU_VERSION",	21,	_SC_XOPEN_XCU_VERSION},
#endif
#ifdef _SC_XOPEN_UNIX
   {"_SC_XOPEN_UNIX",	14,	_SC_XOPEN_UNIX},
#endif
#ifdef _SC_XOPEN_CRYPT
   {"_SC_XOPEN_CRYPT",	15,	_SC_XOPEN_CRYPT},
#endif
#ifdef _SC_XOPEN_ENH_I18N
   {"_SC_XOPEN_ENH_I18N",	18,	_SC_XOPEN_ENH_I18N},
#endif
#ifdef _SC_XOPEN_SHM
   {"_SC_XOPEN_SHM",	13,	_SC_XOPEN_SHM},
#endif
#ifdef _SC_2_CHAR_TERM
   {"_SC_2_CHAR_TERM",	15,	_SC_2_CHAR_TERM},
#endif
#ifdef _SC_2_C_VERSION
   {"_SC_2_C_VERSION",	15,	_SC_2_C_VERSION},
#endif
#ifdef _SC_2_UPE
   {"_SC_2_UPE",	9,	_SC_2_UPE},
#endif
#ifdef _SC_XOPEN_XPG2
   {"_SC_XOPEN_XPG2",	14,	_SC_XOPEN_XPG2},
#endif
#ifdef _SC_XOPEN_XPG3
   {"_SC_XOPEN_XPG3",	14,	_SC_XOPEN_XPG3},
#endif
#ifdef _SC_XOPEN_XPG4
   {"_SC_XOPEN_XPG4",	14,	_SC_XOPEN_XPG4},
#endif
#ifdef _SC_CHAR_BIT
   {"_SC_CHAR_BIT",	12,	_SC_CHAR_BIT},
#endif
#ifdef _SC_CHAR_MAX
   {"_SC_CHAR_MAX",	12,	_SC_CHAR_MAX},
#endif
#ifdef _SC_CHAR_MIN
   {"_SC_CHAR_MIN",	12,	_SC_CHAR_MIN},
#endif
#ifdef _SC_INT_MAX
   {"_SC_INT_MAX",	11,	_SC_INT_MAX},
#endif
#ifdef _SC_INT_MIN
   {"_SC_INT_MIN",	11,	_SC_INT_MIN},
#endif
#ifdef _SC_LONG_BIT
   {"_SC_LONG_BIT",	12,	_SC_LONG_BIT},
#endif
#ifdef _SC_WORD_BIT
   {"_SC_WORD_BIT",	12,	_SC_WORD_BIT},
#endif
#ifdef _SC_MB_LEN_MAX
   {"_SC_MB_LEN_MAX",	14,	_SC_MB_LEN_MAX},
#endif
#ifdef _SC_NZERO
   {"_SC_NZERO",	9,	_SC_NZERO},
#endif
#ifdef _SC_SSIZE_MAX
   {"_SC_SSIZE_MAX",	13,	_SC_SSIZE_MAX},
#endif
#ifdef _SC_SCHAR_MAX
   {"_SC_SCHAR_MAX",	13,	_SC_SCHAR_MAX},
#endif
#ifdef _SC_SCHAR_MIN
   {"_SC_SCHAR_MIN",	13,	_SC_SCHAR_MIN},
#endif
#ifdef _SC_SHRT_MAX
   {"_SC_SHRT_MAX",	12,	_SC_SHRT_MAX},
#endif
#ifdef _SC_SHRT_MIN
   {"_SC_SHRT_MIN",	12,	_SC_SHRT_MIN},
#endif
#ifdef _SC_UCHAR_MAX
   {"_SC_UCHAR_MAX",	13,	_SC_UCHAR_MAX},
#endif
#ifdef _SC_UINT_MAX
   {"_SC_UINT_MAX",	12,	_SC_UINT_MAX},
#endif
#ifdef _SC_ULONG_MAX
   {"_SC_ULONG_MAX",	13,	_SC_ULONG_MAX},
#endif
#ifdef _SC_USHRT_MAX
   {"_SC_USHRT_MAX",	13,	_SC_USHRT_MAX},
#endif
#ifdef _SC_NL_ARGMAX
   {"_SC_NL_ARGMAX",	13,	_SC_NL_ARGMAX},
#endif
#ifdef _SC_NL_LANGMAX
   {"_SC_NL_LANGMAX",	14,	_SC_NL_LANGMAX},
#endif
#ifdef _SC_NL_MSGMAX
   {"_SC_NL_MSGMAX",	13,	_SC_NL_MSGMAX},
#endif
#ifdef _SC_NL_NMAX
   {"_SC_NL_NMAX",	11,	_SC_NL_NMAX},
#endif
#ifdef _SC_NL_SETMAX
   {"_SC_NL_SETMAX",	13,	_SC_NL_SETMAX},
#endif
#ifdef _SC_NL_TEXTMAX
   {"_SC_NL_TEXTMAX",	14,	_SC_NL_TEXTMAX},
#endif
#ifdef _SC_XBS5_ILP32_OFF32
   {"_SC_XBS5_ILP32_OFF32",	20,	_SC_XBS5_ILP32_OFF32},
#endif
#ifdef _SC_XBS5_ILP32_OFFBIG
   {"_SC_XBS5_ILP32_OFFBIG",	21,	_SC_XBS5_ILP32_OFFBIG},
#endif
#ifdef _SC_XBS5_LP64_OFF64
   {"_SC_XBS5_LP64_OFF64",	19,	_SC_XBS5_LP64_OFF64},
#endif
#ifdef _SC_XBS5_LPBIG_OFFBIG
   {"_SC_XBS5_LPBIG_OFFBIG",	21,	_SC_XBS5_LPBIG_OFFBIG},
#endif
#ifdef _SC_XOPEN_LEGACY
   {"_SC_XOPEN_LEGACY",	16,	_SC_XOPEN_LEGACY},
#endif
#ifdef _SC_XOPEN_REALTIME
   {"_SC_XOPEN_REALTIME",	18,	_SC_XOPEN_REALTIME},
#endif
#ifdef _SC_XOPEN_REALTIME_THREADS
   {"_SC_XOPEN_REALTIME_THREADS",	26,	_SC_XOPEN_REALTIME_THREADS},
#endif
#ifdef _SC_ADVISORY_INFO
   {"_SC_ADVISORY_INFO",	17,	_SC_ADVISORY_INFO},
#endif
#ifdef _SC_BARRIERS
   {"_SC_BARRIERS",	12,	_SC_BARRIERS},
#endif
#ifdef _SC_BASE
   {"_SC_BASE",	8,	_SC_BASE},
#endif
#ifdef _SC_C_LANG_SUPPORT
   {"_SC_C_LANG_SUPPORT",	18,	_SC_C_LANG_SUPPORT},
#endif
#ifdef _SC_C_LANG_SUPPORT_R
   {"_SC_C_LANG_SUPPORT_R",	20,	_SC_C_LANG_SUPPORT_R},
#endif
#ifdef _SC_CLOCK_SELECTION
   {"_SC_CLOCK_SELECTION",	19,	_SC_CLOCK_SELECTION},
#endif
#ifdef _SC_CPUTIME
   {"_SC_CPUTIME",	11,	_SC_CPUTIME},
#endif
#ifdef _SC_THREAD_CPUTIME
   {"_SC_THREAD_CPUTIME",	18,	_SC_THREAD_CPUTIME},
#endif
#ifdef _SC_DEVICE_IO
   {"_SC_DEVICE_IO",	13,	_SC_DEVICE_IO},
#endif
#ifdef _SC_DEVICE_SPECIFIC
   {"_SC_DEVICE_SPECIFIC",	19,	_SC_DEVICE_SPECIFIC},
#endif
#ifdef _SC_DEVICE_SPECIFIC_R
   {"_SC_DEVICE_SPECIFIC_R",	21,	_SC_DEVICE_SPECIFIC_R},
#endif
#ifdef _SC_FD_MGMT
   {"_SC_FD_MGMT",	11,	_SC_FD_MGMT},
#endif
#ifdef _SC_FIFO
   {"_SC_FIFO",	8,	_SC_FIFO},
#endif
#ifdef _SC_PIPE
   {"_SC_PIPE",	8,	_SC_PIPE},
#endif
#ifdef _SC_FILE_ATTRIBUTES
   {"_SC_FILE_ATTRIBUTES",	19,	_SC_FILE_ATTRIBUTES},
#endif
#ifdef _SC_FILE_LOCKING
   {"_SC_FILE_LOCKING",	16,	_SC_FILE_LOCKING},
#endif
#ifdef _SC_FILE_SYSTEM
   {"_SC_FILE_SYSTEM",	15,	_SC_FILE_SYSTEM},
#endif
#ifdef _SC_MONOTONIC_CLOCK
   {"_SC_MONOTONIC_CLOCK",	19,	_SC_MONOTONIC_CLOCK},
#endif
#ifdef _SC_MULTI_PROCESS
   {"_SC_MULTI_PROCESS",	17,	_SC_MULTI_PROCESS},
#endif
#ifdef _SC_SINGLE_PROCESS
   {"_SC_SINGLE_PROCESS",	18,	_SC_SINGLE_PROCESS},
#endif
#ifdef _SC_NETWORKING
   {"_SC_NETWORKING",	14,	_SC_NETWORKING},
#endif
#ifdef _SC_READER_WRITER_LOCKS
   {"_SC_READER_WRITER_LOCKS",	23,	_SC_READER_WRITER_LOCKS},
#endif
#ifdef _SC_SPIN_LOCKS
   {"_SC_SPIN_LOCKS",	14,	_SC_SPIN_LOCKS},
#endif
#ifdef _SC_REGEXP
   {"_SC_REGEXP",	10,	_SC_REGEXP},
#endif
#ifdef _SC_REGEX_VERSION
   {"_SC_REGEX_VERSION",	17,	_SC_REGEX_VERSION},
#endif
#ifdef _SC_SHELL
   {"_SC_SHELL",	9,	_SC_SHELL},
#endif
#ifdef _SC_SIGNALS
   {"_SC_SIGNALS",	11,	_SC_SIGNALS},
#endif
#ifdef _SC_SPAWN
   {"_SC_SPAWN",	9,	_SC_SPAWN},
#endif
#ifdef _SC_SPORADIC_SERVER
   {"_SC_SPORADIC_SERVER",	19,	_SC_SPORADIC_SERVER},
#endif
#ifdef _SC_THREAD_SPORADIC_SERVER
   {"_SC_THREAD_SPORADIC_SERVER",	26,	_SC_THREAD_SPORADIC_SERVER},
#endif
#ifdef _SC_SYSTEM_DATABASE
   {"_SC_SYSTEM_DATABASE",	19,	_SC_SYSTEM_DATABASE},
#endif
#ifdef _SC_SYSTEM_DATABASE_R
   {"_SC_SYSTEM_DATABASE_R",	21,	_SC_SYSTEM_DATABASE_R},
#endif
#ifdef _SC_TIMEOUTS
   {"_SC_TIMEOUTS",	12,	_SC_TIMEOUTS},
#endif
#ifdef _SC_TYPED_MEMORY_OBJECTS
   {"_SC_TYPED_MEMORY_OBJECTS",	24,	_SC_TYPED_MEMORY_OBJECTS},
#endif
#ifdef _SC_USER_GROUPS
   {"_SC_USER_GROUPS",	15,	_SC_USER_GROUPS},
#endif
#ifdef _SC_USER_GROUPS_R
   {"_SC_USER_GROUPS_R",	17,	_SC_USER_GROUPS_R},
#endif
#ifdef _SC_2_PBS
   {"_SC_2_PBS",	9,	_SC_2_PBS},
#endif
#ifdef _SC_2_PBS_ACCOUNTING
   {"_SC_2_PBS_ACCOUNTING",	20,	_SC_2_PBS_ACCOUNTING},
#endif
#ifdef _SC_2_PBS_LOCATE
   {"_SC_2_PBS_LOCATE",	16,	_SC_2_PBS_LOCATE},
#endif
#ifdef _SC_2_PBS_MESSAGE
   {"_SC_2_PBS_MESSAGE",	17,	_SC_2_PBS_MESSAGE},
#endif
#ifdef _SC_2_PBS_TRACK
   {"_SC_2_PBS_TRACK",	15,	_SC_2_PBS_TRACK},
#endif
#ifdef _SC_SYMLOOP_MAX
   {"_SC_SYMLOOP_MAX",	15,	_SC_SYMLOOP_MAX},
#endif
#ifdef _SC_STREAMS
   {"_SC_STREAMS",	11,	_SC_STREAMS},
#endif
#ifdef _SC_2_PBS_CHECKPOINT
   {"_SC_2_PBS_CHECKPOINT",	20,	_SC_2_PBS_CHECKPOINT},
#endif
#ifdef _SC_V6_ILP32_OFF32
   {"_SC_V6_ILP32_OFF32",	18,	_SC_V6_ILP32_OFF32},
#endif
#ifdef _SC_V6_ILP32_OFFBIG
   {"_SC_V6_ILP32_OFFBIG",	19,	_SC_V6_ILP32_OFFBIG},
#endif
#ifdef _SC_V6_LP64_OFF64
   {"_SC_V6_LP64_OFF64",	17,	_SC_V6_LP64_OFF64},
#endif
#ifdef _SC_V6_LPBIG_OFFBIG
   {"_SC_V6_LPBIG_OFFBIG",	19,	_SC_V6_LPBIG_OFFBIG},
#endif
#ifdef _SC_HOST_NAME_MAX
   {"_SC_HOST_NAME_MAX",	17,	_SC_HOST_NAME_MAX},
#endif
#ifdef _SC_TRACE
   {"_SC_TRACE",	9,	_SC_TRACE},
#endif
#ifdef _SC_TRACE_EVENT_FILTER
   {"_SC_TRACE_EVENT_FILTER",	22,	_SC_TRACE_EVENT_FILTER},
#endif
#ifdef _SC_TRACE_INHERIT
   {"_SC_TRACE_INHERIT",	17,	_SC_TRACE_INHERIT},
#endif
#ifdef _SC_TRACE_LOG
   {"_SC_TRACE_LOG",	13,	_SC_TRACE_LOG},
#endif
#ifdef _SC_LEVEL1_ICACHE_SIZE
   {"_SC_LEVEL1_ICACHE_SIZE",	22,	_SC_LEVEL1_ICACHE_SIZE},
#endif
#ifdef _SC_LEVEL1_ICACHE_ASSOC
   {"_SC_LEVEL1_ICACHE_ASSOC",	23,	_SC_LEVEL1_ICACHE_ASSOC},
#endif
#ifdef _SC_LEVEL1_ICACHE_LINESIZE
   {"_SC_LEVEL1_ICACHE_LINESIZE",	26,	_SC_LEVEL1_ICACHE_LINESIZE},
#endif
#ifdef _SC_LEVEL1_DCACHE_SIZE
   {"_SC_LEVEL1_DCACHE_SIZE",	22,	_SC_LEVEL1_DCACHE_SIZE},
#endif
#ifdef _SC_LEVEL1_DCACHE_ASSOC
   {"_SC_LEVEL1_DCACHE_ASSOC",	23,	_SC_LEVEL1_DCACHE_ASSOC},
#endif
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
   {"_SC_LEVEL1_DCACHE_LINESIZE",	26,	_SC_LEVEL1_DCACHE_LINESIZE},
#endif
#ifdef _SC_LEVEL2_CACHE_SIZE
   {"_SC_LEVEL2_CACHE_SIZE",	21,	_SC_LEVEL2_CACHE_SIZE},
#endif
#ifdef _SC_LEVEL2_CACHE_ASSOC
   {"_SC_LEVEL2_CACHE_ASSOC",	22,	_SC_LEVEL2_CACHE_ASSOC},
#endif
#ifdef _SC_LEVEL2_CACHE_LINESIZE
   {"_SC_LEVEL2_CACHE_LINESIZE",	25,	_SC_LEVEL2_CACHE_LINESIZE},
#endif
#ifdef _SC_LEVEL3_CACHE_SIZE
   {"_SC_LEVEL3_CACHE_SIZE",	21,	_SC_LEVEL3_CACHE_SIZE},
#endif
#ifdef _SC_LEVEL3_CACHE_ASSOC
   {"_SC_LEVEL3_CACHE_ASSOC",	22,	_SC_LEVEL3_CACHE_ASSOC},
#endif
#ifdef _SC_LEVEL3_CACHE_LINESIZE
   {"_SC_LEVEL3_CACHE_LINESIZE",	25,	_SC_LEVEL3_CACHE_LINESIZE},
#endif
#ifdef _SC_LEVEL4_CACHE_SIZE
   {"_SC_LEVEL4_CACHE_SIZE",	21,	_SC_LEVEL4_CACHE_SIZE},
#endif
#ifdef _SC_LEVEL4_CACHE_ASSOC
   {"_SC_LEVEL4_CACHE_ASSOC",	22,	_SC_LEVEL4_CACHE_ASSOC},
#endif
#ifdef _SC_LEVEL4_CACHE_LINESIZE
   {"_SC_LEVEL4_CACHE_LINESIZE",	25,	_SC_LEVEL4_CACHE_LINESIZE},
#endif
#ifdef _SC_IPV6
   {"_SC_IPV6",	8,	_SC_IPV6},
#endif
#ifdef _SC_RAW_SOCKETS
   {"_SC_RAW_SOCKETS",	15,	_SC_RAW_SOCKETS},
#endif
   {NULL, 0, -1}
};

/*}}}*/

#ifdef HAVE_CONFSTR
static const Name_Map_Type CS_Name_Map_Table[] = /*{{{*/
{
#ifdef _CS_PATH
   {"_CS_PATH",	8,	_CS_PATH},
#endif
#ifdef _CS_V6_WIDTH_RESTRICTED_ENVS
   {"_CS_V6_WIDTH_RESTRICTED_ENVS",	28,	_CS_V6_WIDTH_RESTRICTED_ENVS},
#endif
#ifdef _CS_GNU_LIBC_VERSION
   {"_CS_GNU_LIBC_VERSION",	20,	_CS_GNU_LIBC_VERSION},
#endif
#ifdef _CS_GNU_LIBPTHREAD_VERSION
   {"_CS_GNU_LIBPTHREAD_VERSION",	26,	_CS_GNU_LIBPTHREAD_VERSION},
#endif
#ifdef _CS_LFS_CFLAGS
   {"_CS_LFS_CFLAGS",	14,	_CS_LFS_CFLAGS},
#endif
#ifdef _CS_LFS_LDFLAGS
   {"_CS_LFS_LDFLAGS",	15,	_CS_LFS_LDFLAGS},
#endif
#ifdef _CS_LFS_LIBS
   {"_CS_LFS_LIBS",	12,	_CS_LFS_LIBS},
#endif
#ifdef _CS_LFS_LINTFLAGS
   {"_CS_LFS_LINTFLAGS",	17,	_CS_LFS_LINTFLAGS},
#endif
#ifdef _CS_LFS64_CFLAGS
   {"_CS_LFS64_CFLAGS",	16,	_CS_LFS64_CFLAGS},
#endif
#ifdef _CS_LFS64_LDFLAGS
   {"_CS_LFS64_LDFLAGS",	17,	_CS_LFS64_LDFLAGS},
#endif
#ifdef _CS_LFS64_LIBS
   {"_CS_LFS64_LIBS",	14,	_CS_LFS64_LIBS},
#endif
#ifdef _CS_LFS64_LINTFLAGS
   {"_CS_LFS64_LINTFLAGS",	19,	_CS_LFS64_LINTFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFF32_CFLAGS
   {"_CS_XBS5_ILP32_OFF32_CFLAGS",	27,	_CS_XBS5_ILP32_OFF32_CFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LDFLAGS
   {"_CS_XBS5_ILP32_OFF32_LDFLAGS",	28,	_CS_XBS5_ILP32_OFF32_LDFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LIBS
   {"_CS_XBS5_ILP32_OFF32_LIBS",	25,	_CS_XBS5_ILP32_OFF32_LIBS},
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LINTFLAGS
   {"_CS_XBS5_ILP32_OFF32_LINTFLAGS",	30,	_CS_XBS5_ILP32_OFF32_LINTFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_CFLAGS
   {"_CS_XBS5_ILP32_OFFBIG_CFLAGS",	28,	_CS_XBS5_ILP32_OFFBIG_CFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LDFLAGS
   {"_CS_XBS5_ILP32_OFFBIG_LDFLAGS",	29,	_CS_XBS5_ILP32_OFFBIG_LDFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LIBS
   {"_CS_XBS5_ILP32_OFFBIG_LIBS",	26,	_CS_XBS5_ILP32_OFFBIG_LIBS},
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LINTFLAGS
   {"_CS_XBS5_ILP32_OFFBIG_LINTFLAGS",	31,	_CS_XBS5_ILP32_OFFBIG_LINTFLAGS},
#endif
#ifdef _CS_XBS5_LP64_OFF64_CFLAGS
   {"_CS_XBS5_LP64_OFF64_CFLAGS",	26,	_CS_XBS5_LP64_OFF64_CFLAGS},
#endif
#ifdef _CS_XBS5_LP64_OFF64_LDFLAGS
   {"_CS_XBS5_LP64_OFF64_LDFLAGS",	27,	_CS_XBS5_LP64_OFF64_LDFLAGS},
#endif
#ifdef _CS_XBS5_LP64_OFF64_LIBS
   {"_CS_XBS5_LP64_OFF64_LIBS",	24,	_CS_XBS5_LP64_OFF64_LIBS},
#endif
#ifdef _CS_XBS5_LP64_OFF64_LINTFLAGS
   {"_CS_XBS5_LP64_OFF64_LINTFLAGS",	29,	_CS_XBS5_LP64_OFF64_LINTFLAGS},
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_CFLAGS
   {"_CS_XBS5_LPBIG_OFFBIG_CFLAGS",	28,	_CS_XBS5_LPBIG_OFFBIG_CFLAGS},
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LDFLAGS
   {"_CS_XBS5_LPBIG_OFFBIG_LDFLAGS",	29,	_CS_XBS5_LPBIG_OFFBIG_LDFLAGS},
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LIBS
   {"_CS_XBS5_LPBIG_OFFBIG_LIBS",	26,	_CS_XBS5_LPBIG_OFFBIG_LIBS},
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS
   {"_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS",	31,	_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS},
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_CFLAGS
   {"_CS_POSIX_V6_ILP32_OFF32_CFLAGS",	31,	_CS_POSIX_V6_ILP32_OFF32_CFLAGS},
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LDFLAGS
   {"_CS_POSIX_V6_ILP32_OFF32_LDFLAGS",	32,	_CS_POSIX_V6_ILP32_OFF32_LDFLAGS},
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LIBS
   {"_CS_POSIX_V6_ILP32_OFF32_LIBS",	29,	_CS_POSIX_V6_ILP32_OFF32_LIBS},
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS
   {"_CS_POSIX_V6_ILP32_OFF32_LINTFLAGS",	34,	_CS_POSIX_V6_ILP32_OFF32_LINTFLAGS},
#endif
#ifdef _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS
   {"_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS",	32,	_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS},
#endif
#ifdef _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS
   {"_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS",	33,	_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS},
#endif
#ifdef _CS_POSIX_V6_ILP32_OFFBIG_LIBS
   {"_CS_POSIX_V6_ILP32_OFFBIG_LIBS",	30,	_CS_POSIX_V6_ILP32_OFFBIG_LIBS},
#endif
#ifdef _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS
   {"_CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS",	35,	_CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS},
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_CFLAGS
   {"_CS_POSIX_V6_LP64_OFF64_CFLAGS",	30,	_CS_POSIX_V6_LP64_OFF64_CFLAGS},
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_LDFLAGS
   {"_CS_POSIX_V6_LP64_OFF64_LDFLAGS",	31,	_CS_POSIX_V6_LP64_OFF64_LDFLAGS},
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_LIBS
   {"_CS_POSIX_V6_LP64_OFF64_LIBS",	28,	_CS_POSIX_V6_LP64_OFF64_LIBS},
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_LINTFLAGS
   {"_CS_POSIX_V6_LP64_OFF64_LINTFLAGS",	33,	_CS_POSIX_V6_LP64_OFF64_LINTFLAGS},
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS
   {"_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS",	32,	_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS},
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS
   {"_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS",	33,	_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS},
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_LIBS
   {"_CS_POSIX_V6_LPBIG_OFFBIG_LIBS",	30,	_CS_POSIX_V6_LPBIG_OFFBIG_LIBS},
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS
   {"_CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS",	35,	_CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS},
#endif
   {NULL, 0, -1}
};

/*}}}*/
#endif

#ifdef HAVE_PATHCONF
static const Name_Map_Type PC_Name_Map_Table[] = /*{{{*/
{
#ifdef _PC_LINK_MAX
   {"_PC_LINK_MAX",	12,	_PC_LINK_MAX},
#endif
#ifdef _PC_MAX_CANON
   {"_PC_MAX_CANON",	13,	_PC_MAX_CANON},
#endif
#ifdef _PC_MAX_INPUT
   {"_PC_MAX_INPUT",	13,	_PC_MAX_INPUT},
#endif
#ifdef _PC_NAME_MAX
   {"_PC_NAME_MAX",	12,	_PC_NAME_MAX},
#endif
#ifdef _PC_PATH_MAX
   {"_PC_PATH_MAX",	12,	_PC_PATH_MAX},
#endif
#ifdef _PC_PIPE_BUF
   {"_PC_PIPE_BUF",	12,	_PC_PIPE_BUF},
#endif
#ifdef _PC_CHOWN_RESTRICTED
   {"_PC_CHOWN_RESTRICTED",	20,	_PC_CHOWN_RESTRICTED},
#endif
#ifdef _PC_NO_TRUNC
   {"_PC_NO_TRUNC",	12,	_PC_NO_TRUNC},
#endif
#ifdef _PC_VDISABLE
   {"_PC_VDISABLE",	12,	_PC_VDISABLE},
#endif
#ifdef _PC_SYNC_IO
   {"_PC_SYNC_IO",	11,	_PC_SYNC_IO},
#endif
#ifdef _PC_ASYNC_IO
   {"_PC_ASYNC_IO",	12,	_PC_ASYNC_IO},
#endif
#ifdef _PC_PRIO_IO
   {"_PC_PRIO_IO",	11,	_PC_PRIO_IO},
#endif
#ifdef _PC_SOCK_MAXBUF
   {"_PC_SOCK_MAXBUF",	15,	_PC_SOCK_MAXBUF},
#endif
#ifdef _PC_FILESIZEBITS
   {"_PC_FILESIZEBITS",	16,	_PC_FILESIZEBITS},
#endif
#ifdef _PC_REC_INCR_XFER_SIZE
   {"_PC_REC_INCR_XFER_SIZE",	22,	_PC_REC_INCR_XFER_SIZE},
#endif
#ifdef _PC_REC_MAX_XFER_SIZE
   {"_PC_REC_MAX_XFER_SIZE",	21,	_PC_REC_MAX_XFER_SIZE},
#endif
#ifdef _PC_REC_MIN_XFER_SIZE
   {"_PC_REC_MIN_XFER_SIZE",	21,	_PC_REC_MIN_XFER_SIZE},
#endif
#ifdef _PC_REC_XFER_ALIGN
   {"_PC_REC_XFER_ALIGN",	18,	_PC_REC_XFER_ALIGN},
#endif
#ifdef _PC_ALLOC_SIZE_MIN
   {"_PC_ALLOC_SIZE_MIN",	18,	_PC_ALLOC_SIZE_MIN},
#endif
#ifdef _PC_SYMLINK_MAX
   {"_PC_SYMLINK_MAX",	15,	_PC_SYMLINK_MAX},
#endif
#ifdef _PC_2_SYMLINKS
   {"_PC_2_SYMLINKS",	14,	_PC_2_SYMLINKS},
#endif
   {NULL, 0, -1},
};

/*}}}*/
#endif

/* return -1 upon error, 0 if name not found, 1 if found or specified. */
static int pop_iname (const Name_Map_Type *map, int *inamep)
{
   if (SLang_peek_at_stack () == SLANG_STRING_TYPE)
     {
	char *name;
	if (-1 == SLang_pop_slstring (&name))
	  return -1;
	map = lookup_name (map, name);
	SLang_free_slstring (name);
	if (map == NULL)
	  return 0;

	*inamep = map->iname;
     }
   else if (-1 == SLang_pop_int (inamep))
     return -1;

   return 1;
}

/* Usage: val = sysconf (name|iname [,defval]) */
static void sysconf_intrinsic (void)
{
   int has_def_val = 0;
   long defval = -1;
   long val;
   int iname;
   int status;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_long (&defval))
	  return;
	has_def_val = 1;
     }

   status = pop_iname (SC_Name_Map_Table, &iname);
   if (status == -1)
     return;

   if (status == 0)
     goto return_default;

   errno = 0;
   val = sysconf (iname);
   if (val == -1)
     {
	if (errno)
	  goto return_default;

	if (has_def_val)
	  val = defval;
     }

   (void) SLang_push_long (val);
   return;

return_default:
   if (has_def_val)
     (void) SLang_push_long (defval);
   else
     (void) SLang_push_null ();
}

#ifdef HAVE_PATHCONF
static void pathconf_intrinsic (void)
{
   int has_def_val = 0;
   long defval = -1;
   long val;
   int iname;
   int status;
   char *path = NULL;
   int fd = -1, e;

   if (SLang_Num_Function_Args == 3)
     {
	if (-1 == SLang_pop_long (&defval))
	  return;
	has_def_val = 1;
     }

   status = pop_iname (PC_Name_Map_Table, &iname);
   if (status == -1)
     return;

   switch (SLang_peek_at_stack ())
     {
      case SLANG_STRING_TYPE:
	if (-1 == SLang_pop_slstring (&path))
	  return;
	break;

      case SLANG_FILE_PTR_TYPE:
	  {
	     FILE *fp;
	     SLang_MMT_Type *mmt;

	     if (-1 == SLang_pop_fileptr (&mmt, &fp))
	       return;
	     fd = fileno (fp);
	     SLang_free_mmt (mmt);
	  }
	break;

      default:
	  {
	     SLFile_FD_Type *f;
	     if (-1 == SLfile_pop_fd (&f))
	       return;
	     if (-1 == SLfile_get_fd (f, &fd))
	       {
		  SLfile_free_fd (f);
		  return;
	       }
	     SLfile_free_fd (f);
	  }
     }

   if (status == 0)
     {
	e = EINVAL;
	if (path != NULL)
	  SLang_free_slstring (path);
	goto return_default;
     }

   errno = 0;
   if (path != NULL)
     {
	val = pathconf (path, iname);
	e = errno;
	SLang_free_slstring (path);
     }
   else
     {
	val = fpathconf (fd, iname);
	e = errno;
     }

   if (val == -1)
     {
	if (e)
	  goto return_default;

	if (has_def_val)
	  val = defval;
     }

   (void) SLang_push_long (val);
   return;

return_default:
   if (has_def_val && (e == EINVAL))
     (void) SLang_push_long (defval);
   else
     {
	SLerrno_set_errno (e);
	(void) SLang_push_null ();
     }
}
#endif

#ifdef HAVE_CONFSTR
/* Usage: val = sysconf (name|iname [,defval]) */
static void confstr_intrinsic (void)
{
   char *defstr = NULL;
   int use_def_val = 0;
   int iname;
   int status;
   char buf[10];
   char *bufstr;
   size_t len;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_slstring (&defstr))
	  return;
	use_def_val = 1;
     }

   status = pop_iname (CS_Name_Map_Table, &iname);
   if (status == -1)
     {
	SLang_free_slstring (defstr);  /* NULL ok */
	return;
     }

   errno = 0;
   if ((status == 0)
       || (0 == (len = confstr (iname, buf, sizeof(buf)))))
     {
	if ((errno != EINVAL) && (errno != 0))
	  {
	     SLerrno_set_errno (errno);
	     use_def_val = 0;
	  }

	if (use_def_val)
	  (void) SLang_push_string(defstr);
	else
	  (void) SLang_push_null ();

	SLang_free_slstring (defstr);
	return;
     }

   SLang_free_slstring (defstr);
   defstr = NULL;

   if (len <= sizeof(buf))
     {
	SLang_push_string (buf);
	return;
     }

   if (NULL == (bufstr = SLmalloc (len)))
     return;

   errno = 0;
   len = confstr (iname, bufstr, len);
   if (len == 0)
     {
	SLerrno_set_errno (errno);
	SLfree (bufstr);
	(void) SLang_push_null ();
	return;
     }
   (void) SLang_push_malloced_string (bufstr);   /* frees bufstr */
}
#endif

static int push_map (const Name_Map_Type *map)
{
   SLindex_Type num = 0;
   const Name_Map_Type *m;
   SLang_Array_Type *at;
   char **names;

   m = map;
   while (m->name != NULL)
     {
	num++;
	m++;
     }

   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 1, NULL, &num, 1)))
     return -1;

   names = (char **) at->data;
   m = map;
   while (m->name != NULL)
     {
	if (NULL == (*names = SLang_create_slstring ((char *) m->name)))
	  {
	     SLang_free_array (at);
	     return -1;
	  }
	names++;
	m++;
     }

   return SLang_push_array (at, 1);
}

static void sysconf_names_intrinsic (void)
{
   (void) push_map (SC_Name_Map_Table);
}

#ifdef HAVE_PATHCONF
static void pathconf_names_intrinsic (void)
{
   (void) push_map (PC_Name_Map_Table);
}
#endif

#ifdef HAVE_CONFSTR
static void confstr_names_intrinsic (void)
{
   (void) push_map (CS_Name_Map_Table);
}
#endif

static SLang_Intrin_Fun_Type Module_Intrinsics [] =
{
   MAKE_INTRINSIC_0("sysconf", sysconf_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("sysconf_names", sysconf_names_intrinsic, SLANG_VOID_TYPE),
#ifdef HAVE_PATHCONF
   MAKE_INTRINSIC_0("pathconf", pathconf_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("pathconf_names", pathconf_names_intrinsic, SLANG_VOID_TYPE),
#endif
#ifdef HAVE_CONFSTR
   MAKE_INTRINSIC_0("confstr", confstr_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("confstr_names", confstr_names_intrinsic, SLANG_VOID_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

int init_sysconf_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (-1 == SLns_add_intrin_fun_table (ns, Module_Intrinsics, NULL))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_sysconf_module (void)
{
}
