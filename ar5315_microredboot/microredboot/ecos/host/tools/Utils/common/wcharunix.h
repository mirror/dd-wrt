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

// ----------------------------------------------------------------------------
// This file defines the generic-text routine mappings found in TCHAR.H on windows
// ----------------------------------------------------------------------------

#define _TEOF       EOF

#define __T(x)      x


/* Program */

#define _tmain      main
#define _tWinMain   WinMain
#define _tenviron   environ
#define __targv     _argv


/* Formatted i/o */

#define _tprintf    printf
#define _ftprintf   fprintf
#define _stprintf   sprintf
#define _sntprintf  snprintf
#define _vtprintf   vprintf
#define _vftprintf  vfprintf
#define _vstprintf  vsprintf
#define _vsntprintf vsnprintf
#define _tscanf     scanf
#define _ftscanf    fscanf
#define _stscanf    sscanf


/* Unformatted i/o */

#define _fgettc     fgetc
#define _fgettchar  fgetchar
#define _fgetts     fgets
#define _fputtc     fputc
#define _fputtchar  fputchar
#define _fputts     fputs
#define _gettc      getc
#define _gettchar   getchar
#define _getts      gets
#define _puttc      putc
#define _puttchar   putchar
#define _putts      puts
#define _ungettc    ungetc


/* String conversion functions */

#define _tcstod     strtod
#define _tcstol     strtol
#define _tcstoul    strtoul

#define _itot       itoa
#define _ltot       ltoa
#define _ultot      ultoa
#define _ttoi       atoi
#define _ttol       atol

#define _ttoi64     atoi64
#define _i64tot     i64toa
#define _ui64tot    ui64toa

/* String functions */

/* Note that _mbscat, _mbscpy and _mbsdup are functionally equivalent to 
   strcat, strcpy and strdup, respectively. */

#define _tcscat     strcat
#define _tcscpy     strcpy
#define _tcsdup     strdup

#define _tcslen     strlen
#define _tcsxfrm    strxfrm


/* Execute functions */

#define _texecl     execl
#define _texecle    execle
#define _texeclp    execlp
#define _texeclpe   execlpe
#define _texecv     execv
#define _texecve    execve
#define _texecvp    execvp
#define _texecvpe   execvpe

#define _tspawnl    spawnl
#define _tspawnle   spawnle
#define _tspawnlp   spawnlp
#define _tspawnlpe  spawnlpe
#define _tspawnv    spawnv
#define _tspawnve   spawnve
#define _tspawnvp   spawnvp
#define _tspawnvpe  spawnvpe

#define _tsystem    system


/* Time functions */

#define _tasctime   asctime
#define _tctime     ctime
#define _tstrdate   strdate
#define _tstrtime   strtime
#define _tutime     utime
#define _tcsftime   strftime


/* Directory functions */

#define _tchdir     chdir
#define _tgetcwd    getcwd
#define _tgetdcwd   getdcwd
#define _tmkdir(x)  mkdir(x,00700)
#define _trmdir     rmdir


/* Environment/Path functions */

#define _tfullpath  fullpath
#define _tgetenv    getenv
#define _tmakepath  makepath
// Yuck - /usr/include/stdlib.h defines it as char*, not const char*
#define _tputenv(s) putenv((char*)(s.c_str()))
#define _tsearchenv searchenv
#define _tsplitpath splitpath


/* Stdio functions */

#define _tfdopen    fdopen
#define _tfsopen    fsopen
#define _tfopen     fopen
#define _tfreopen   freopen
#define _tperror    perror
#define _tpopen     popen
#define _ttempnam   tempnam
#define _ttmpnam    tmpnam


/* Io functions */

#define _tchmod     chmod
#define _tcreat     creat
#define _tfindfirst findfirst
#define _tfindfirsti64  findfirsti64
#define _tfindnext  findnext
#define _tfindnexti64   findnexti64
#define _tmktemp    mktemp

#define _topen      open
#define _taccess    access

#define _tremove    remove
#define _trename    rename
#define _tsopen     sopen
#define _tunlink    unlink

#define _tfinddata_t    finddata_t
#define _tfinddatai64_t finddatai64_t


/* ctype functions */

#define _istascii   isascii
#define _istcntrl   iscntrl
#define _istxdigit  isxdigit


/* Stat functions */

#define _tstat      stat
#define _tstati64   stati64



/* ++++++++++++++++++++ SBCS ++++++++++++++++++++ */


typedef char            _TCHAR;
typedef signed char     _TSCHAR;
typedef unsigned char   _TUCHAR;
typedef char            _TXCHAR;
typedef int             _TINT;

/* String functions */

#define _tcschr     strchr
#define _tcscspn    strcspn
#define _tcsncat    strncat
#define _tcsncpy    strncpy
#define _tcspbrk    strpbrk
#define _tcsrchr    strrchr
#define _tcsspn     strspn
#define _tcsstr     strstr
#define _tcstok     strtok

#define _tcsnset    strnset
#define _tcsrev     strrev
#define _tcsset     strset

#define _tcscmp     strcmp
#define _tcsicmp    strcasecmp
#define _tcsnccmp   strncmp
#define _tcsncmp    strncmp
#define _tcsncicmp  strnicmp
#define _tcsnicmp   strncasecmp

#define _tcscoll    strcoll
#define _tcsicoll   stricoll
#define _tcsnccoll  strncoll
#define _tcsncoll   strncoll
#define _tcsncicoll strnicoll
#define _tcsnicoll  strnicoll


/* "logical-character" mappings */

#define _tcsclen    strlen
#define _tcsnccat   strncat
#define _tcsnccpy   strncpy
#define _tcsncset   strnset


/* MBCS-specific functions */

#define _tcsdec     strdec
#define _tcsinc     strinc
#define _tcsnbcnt   strncnt
#define _tcsnccnt   strncnt
#define _tcsnextc   strnextc
#define _tcsninc    strninc
#define _tcsspnp    strspnp

#define _tcslwr     strlwr
#define _tcsupr     strupr
#define _tcsxfrm    strxfrm

#define _istlead(_c)    (0)
#define _istleadbyte(_c)    (0)

/* ctype-functions */

#define _istalnum   isalnum
#define _istalpha   isalpha
#define _istdigit   isdigit
#define _istgraph   isgraph
#define _istlower   islower
#define _istprint   isprint
#define _istpunct   ispunct
#define _istspace   isspace
#define _istupper   isupper

#define _totupper   toupper
#define _totlower   tolower

#define _istlegal(_c)   (1)

typedef char *LPTSTR;
typedef const char *LPCTSTR;
typedef char TCHAR;

#define _TCHAR(x) x
#define _T(x) x

