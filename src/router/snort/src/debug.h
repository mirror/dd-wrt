/* $Id$ */
/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#ifndef DEBUG_H
#define DEBUG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if !defined(INLINE)
#ifdef WIN32
#define INLINE __inline
#else /* WIN32 */
#define INLINE inline
#endif /* WIN32 */
#endif /* !def INLINE */

#include <ctype.h>
#ifdef HAVE_WCHAR_H
/* ISOC99 is defined to get required prototypes */
#ifndef __USE_ISOC99
#define __USE_ISOC99
#endif
#include <wchar.h>
#endif

#define DEBUG_VARIABLE "SNORT_DEBUG"

#define DEBUG_ALL             0xffffffff  /* 4294967295 */
#define DEBUG_INIT            0x00000001  /* 1 */
#define DEBUG_CONFIGRULES     0x00000002  /* 2 */
#define DEBUG_PLUGIN          0x00000004  /* 4 */
#define DEBUG_DATALINK        0x00000008  /* 8 */
//#define DEBUG_IP              0x00000010  /* 16 */
//#define DEBUG_TCPUDP          0x00000020  /* 32 */
#define DEBUG_DECODE          0x00000040  /* 64 */
#define DEBUG_LOG             0x00000080  /* 128 */
#define DEBUG_MSTRING         0x00000100  /* 256 */
#define DEBUG_PARSER          0x00000200  /* 512 */
#define DEBUG_PLUGBASE        0x00000400  /* 1024 */
#define DEBUG_RULES           0x00000800  /* 2048 */
#define DEBUG_FLOW            0x00001000  /* 4096 */
#define DEBUG_STREAM          0x00002000  /* 8192 */
#define DEBUG_PATTERN_MATCH   0x00004000  /* 16384 */
#define DEBUG_DETECT          0x00008000  /* 32768 */
#define DEBUG_SKYPE           0x00010000  /* 65536 */
#define DEBUG_FRAG            0x00020000  /* 131072 */
#define DEBUG_HTTP_DECODE     0x00040000  /* 262144 */
//#define DEBUG_PORTSCAN2       0x00080000  /* 524288 / (+ conv2 ) 589824 */
#define DEBUG_RPC             0x00100000  /* 1048576 */
//#define DEBUG_FLOWSYS         0x00200000  /* 2097152 */
#define DEBUG_HTTPINSPECT     0x00400000  /* 4194304 */
#define DEBUG_STREAM_STATE    0x00800000  /* 8388608 */
#define DEBUG_ASN1            0x01000000  /* 16777216 */
#define DEBUG_FTPTELNET       0x02000000  /* 33554432 */
#define DEBUG_SMTP            0x04000000  /* 67108864 */
//#define DEBUG_DCERPC          0x08000000  /* 134217728 */
#define DEBUG_DNS             0x10000000  /* 268435456 */
#define DEBUG_ATTRIBUTE       0x20000000  /* 536870912 */
#define DEBUG_PORTLISTS       0x40000000  /* 1073741824 */
#define DEBUG_SSL             0x80000000  /* 2147483648 */

void DebugMessageFunc(int dbg,char *fmt, ...);
#ifdef HAVE_WCHAR_H
void DebugWideMessageFunc(int dbg,wchar_t *fmt, ...);
#endif

#ifdef DEBUG

    extern char *DebugMessageFile;
    extern int DebugMessageLine;

    #define    DebugMessage    DebugMessageFile = __FILE__; DebugMessageLine = __LINE__; DebugMessageFunc
    #define    DebugWideMessage    DebugMessageFile = __FILE__; DebugMessageLine = __LINE__; DebugWideMessageFunc

    int GetDebugLevel (void);
    int DebugThis(int level);
#else 

#ifdef WIN32
/* Visual C++ uses the keyword "__inline" rather than "__inline__" */
         #define __inline__ __inline
#endif

#endif /* DEBUG */


#ifdef DEBUG
#define DEBUG_WRAP(code) code
void DebugMessageFunc(int dbg,char *fmt, ...);
#ifdef HAVE_WCHAR_H
void DebugWideMessageFunc(int dbg,wchar_t *fmt, ...);
#endif
#else
#define DEBUG_WRAP(code)
/* I would use DebugMessage(dbt,fmt...) but that only works with GCC */

#endif

#endif /* DEBUG_H */
