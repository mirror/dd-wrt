/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *   Copyright (c) 2013 I2SE GmbH
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/
 
/*====================================================================*
 *
 *   signed slac_debug (struct session * session, signed status, char const * string, char const * format, ...);
 *
 *   slac.h
 *
 *   variation of the GNU error() function that accepts a message in
 *   place of an error code and always returns -1;
 *
 *--------------------------------------------------------------------*/

#ifndef SLAC_DEBUG_SOURCE
#define SLAC_DEBUG_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../iso15118/slac.h"
#include "../tools/types.h"
#include "../tools/error.h"
#include "../tools/flags.h"

#ifdef __GNUC__

__attribute__ ((format (printf, 4, 5)))

#endif

signed slac_debug (struct session * session, signed status, char const * string, char const * format, ...)

{
	extern char const * program_name;
	
	if (_allclr (session->flags, SLAC_SILENCE))
	{
		if ((program_name) && (* program_name))
		{
			fprintf (stderr, "%s: ", program_name);
		}
		if ((string) && (* string))
		{
			fprintf (stderr, "%s: ", string);
		}
		if ((format) && (*format))
		{
			va_list arglist;
			va_start (arglist, format);
			vfprintf (stderr, format, arglist);
			va_end (arglist);
		}
		fprintf (stderr, "\n");
		fflush (stderr);
	}
	if (status)
	{
		exit (status);
	}
	return (-1);
}


#endif

