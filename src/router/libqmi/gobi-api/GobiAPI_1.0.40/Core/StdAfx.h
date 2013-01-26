/*===========================================================================
FILE: 
   StdAfx.h

DESCRIPTION:
   Application Framework eXtenstions for Linux

PUBLIC CLASSES AND FUNCTIONS:
   
Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <fstream>
#include <assert.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <algorithm>
#include <limits.h>
#include <dirent.h>
#include <sstream>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

//---------------------------------------------------------------------------
// Macro defination
//---------------------------------------------------------------------------

#define ASSERT( x ) assert( x )

#ifdef DEBUG
   #define TRACE printf
#else
   #define TRACE(...)
#endif

//---------------------------------------------------------------------------
// data type defination
//---------------------------------------------------------------------------
#ifndef FALSE
#define FALSE              0
#endif

#ifndef TRUE
#define TRUE               1
#endif

#ifndef CONST
#define CONST              const
#endif

typedef void               VOID;
typedef unsigned long      DWORD;
typedef int                BOOL;   
typedef unsigned char      BYTE;
typedef unsigned short     WORD;
typedef float              FLOAT;
typedef long long          LONGLONG;
typedef unsigned long long ULONGLONG;
typedef signed char        INT8;
typedef double             DOUBLE;

typedef int                INT;
typedef unsigned int       UINT;   
typedef unsigned int *     PUINT;
typedef INT                HANDLE;
typedef HANDLE             HMODULE;

typedef char               CHAR;
typedef short              SHORT;
typedef long               LONG;

typedef unsigned long      ULONG;
typedef ULONG *            PULONG;
typedef unsigned short     USHORT;
typedef USHORT *           PUSHORT;
typedef unsigned char      UCHAR;
typedef UCHAR *            PUCHAR;
typedef char *             PSZ;


typedef CONST CHAR *       LPCSTR;
typedef CHAR *             LPSTR;

typedef BYTE *             PBYTE;
typedef BOOL *             PBOOL;
typedef INT *              PINT;
typedef UINT *             LPINT;
typedef WORD *             PWORD;
typedef PWORD              LPWORD;      
typedef LONG *             LPLONG;
typedef DWORD *            PDWORD;
typedef VOID *             PVOID;
typedef PVOID              LPVOID;      
typedef const void *       LPCVOID;

typedef size_t             SIZE_T;
typedef double             DATE;

// Error code
#define NO_ERROR                    0L
#define ERROR_SUCCESS               0L
#define ERROR_NO_MORE_ITEMS         259L
#define ERROR_CRC                   23L
#define ERROR_OUTOFMEMORY           14L
#define ERROR_CAN_NOT_COMPLETE      1003L
#define ERROR_REVISION_MISMATCH     1306L
#define ERROR_BAD_ARGUMENTS         160L
#define INVALID_SET_FILE_POINTER    -1
#define VALID_HANDLE_VALUE           0
#define INVALID_HANDLE_VALUE        -1
#define INVALID_FILE_SZ             -1

#define ERROR_GEN_FAILURE           31L
#define ERROR_FILE_NOT_FOUND        2L
#define ERROR_NOT_ENOUGH_MEMORY     8L
#define ERROR_INVALID_PARAMETER     87L
#define ERROR_BAD_FORMAT            11L


// Other Constant definitions
#define MAX_PATH                    512
#define INFINITE                    0xffffffff


// SIOCIWFIRSTPRIV = 0x8BE0

// Device I/O control code for setting QMI service
#define QMI_GET_SERVICE_FILE_IOCTL 0x8BE0 + 1

// Device I/O control code for obtaining device VIDPID
#define QMI_GET_VIDPID_IOCTL 0x8BE0 + 2

// Device I/O control code for obtaining device MEID
#define QMI_GET_MEID_IOCTL 0x8BE0 + 3

// Define the directions for pipes
#define READING 0
#define WRITING 1
