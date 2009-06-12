//####COPYRIGHTBEGIN####
//
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This program is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//
//####COPYRIGHTEND####
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   Standard include file - include file
// Usage:
//
//####DESCRIPTIONEND####

// ----------------------------------------------------------------------------
// This header sets us up with what is generally needed, both for WIN32 and UNIX
// Apart from header includes it defines the time-related quantities:
//     Time     - type to measure an absolute time
//     Duration - type to measure the difference between two times, or a delay
//     Now()    - the time now.
//     MIN and MAX
//     LogFunc  - a function to which output can be sent
// ----------------------------------------------------------------------------

#if !defined(AFX_STDAFX_H__F20BA9C4_CFD5_11D2_BF75_00A0C949ADAC__INCLUDED_)
#define AFX_STDAFX_H__F20BA9C4_CFD5_11D2_BF75_00A0C949ADAC__INCLUDED_

#if defined(__CYGWIN__)
  #include <winsock2.h>
  #include <winsock.h>
  #include <unistd.h>
  #include <process.h>
  #include <signal.h>
  #define cPathsep '/'

  #include <malloc.h>     // malloc
  #include <stdlib.h>     // atoi
  #include <errno.h>
  #define WOULDBLOCK WSAEWOULDBLOCK

  #include "wcharunix.h"
  #include <sys/time.h>

  #define _stat stat

  typedef long long Time;
  #define MODE_TEXT

  #include <windows.h>

#elif defined(_WIN32)
  #ifdef _UNICODE
    #ifndef UNICODE
      #define UNICODE         // UNICODE is used by Windows headers
    #endif
  #endif

  #include <tchar.h>
  #undef NDEBUG

  #if _MSC_VER > 1000
    #pragma once
  #endif // _MSC_VER > 1000

  #define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

  #include <winsock2.h>
  #ifndef _WINDOWS_
    #include <windows.h>
  #endif
  #include <direct.h>
  #define cPathsep _TCHAR('\\')
  #include <io.h>
  #include <process.h>

  #include <malloc.h> // _heapchk

  #define CALLBACK    __stdcall               // Calling conventions for a callback
  #define WOULDBLOCK WSAEWOULDBLOCK           // "Would blocking" error
  #define errno       (*_errno())
  #define vsnprintf _vsnprintf
  #pragma warning (disable:4710)  // Not inlined warning
  typedef __int64 Time;
  #define MODE_TEXT _T("t")
#else // UNIX
  #include <termios.h>
  #include <unistd.h>
  #include <sys/wait.h>
  #include <dirent.h>
  #include <sys/file.h>
  #include <sys/socket.h> // socket etc...
  #include <netinet/in.h> // inet_addr
  #include <arpa/inet.h>  // inet_addr
  #include <netdb.h>      // gethostbyname
  #include <sys/timeb.h>
  #include <signal.h>
  #define cPathsep '/'

  #include <malloc.h>     // malloc
  #include <stdlib.h>     // atoi
  #include <errno.h>
  #define WOULDBLOCK EWOULDBLOCK
  #define CALLBACK

  #include "wcharunix.h"
  #include <sys/time.h>

  #define _stat stat

  typedef long long Time;
  #define MODE_TEXT
#endif

#define ECOS_VERSION "2.net"

typedef int Duration;

extern Time Now();

// do not use macros, which would lead to double argument evaluation:
extern int MIN(int a, int b);
extern int MAX(int a, int b);

#include <string.h>

#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>     // vsnprintf

#include <assert.h> // assert
#include <string.h> // strcpy
#include <stdarg.h>
#include <stdio.h>

// Allow user to define a function to which logged output is sent (in addition to being stored internally)
typedef void (CALLBACK LogFunc)(void *, LPCTSTR );

#endif
