/*====================================================================*
 *      
 *   Copyright (c) 2011 by Qualcomm Atheros.
 *   
 *   Permission to use, copy, modify, and/or distribute this software 
 *   for any purpose with or without fee is hereby granted, provided 
 *   that the above copyright notice and this permission notice appear 
 *   in all copies.
 *   
 *   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL 
 *   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL  
 *   THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR 
 *   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 *   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
 *   NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 *   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *   
 *--------------------------------------------------------------------*/

/*====================================================================*

 *
 *   unistd.h - substitute unistd.h file for Windows;
 *
 *   this is an important POSIX header that Microsoft ommits; 
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *	Abdel Younes <younes@leacom.fr>
 *
 *--------------------------------------------------------------------*/

#ifndef UNISTD_HEADER
#define UNISTD_HEADER
 
/*====================================================================*
 *   system header files
 *--------------------------------------------------------------------*/

#if defined (WIN32)
#include <io.h>
#endif

 /*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#if defined (WIN32)
#define sleep(x) Sleep(1000*(x))
#define strcasecmp(a,b) stricmp(a,b)
typedef signed ssize_t;
#endif
 
#if !defined (_MSC_VER) || _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif

#define snprintf _snprintf

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif
 

