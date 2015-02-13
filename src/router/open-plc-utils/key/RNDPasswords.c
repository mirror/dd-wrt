/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
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
 *   void  RNDPasswords (unsigned vendor, unsigned device, unsigned number, unsigned count, unsigned group, unsigned space, flag_t flags);
 *
 *   keys.h
 *
 *   print a number of device address/password pairs on stdout; print
 *   an optional usage flag in the first column for PTS compatability;
 *
 *   vendor is the 24-bit OUI expressed as an integer; device is the
 *   24-bit starting unit address expressed as an integer; number is
 *   the number of address/password pairs to generate; count is the
 *   number of letters/numbers in the password excluding delimiters;
 *   group is the number of letters or numbers appear between space
 *   characters; space characters are suppressed when group is zero
 *   or greater than count;
 *
 *--------------------------------------------------------------------*/

#ifndef RNDPASSWORDS_SOURCE
#define RNDPASSWORDS_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "../tools/types.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/flags.h"
#include "../key/keys.h"

void RNDPasswords (unsigned vendor, unsigned device, unsigned number, unsigned count, unsigned group, char space, flag_t flags)

{
	if (vendor >> 24)
	{
		return;
	}
	if (device >> 24)
	{
		return;
	}
	if (number >> 24)
	{
		return;
	}
	while (number--)
	{
		char buffer [256];
		if (_anyset (flags, PASSWORD_VERBOSE))
		{
			putc ('0', stdout);
			putc (' ', stdout);
		}
		if (_allclr (flags, PASSWORD_SILENCE))
		{
			printf ("%06X", vendor & 0x00FFFFFF);
			printf ("%06X", device & 0x00FFFFFF);
			putc (' ', stdout);
		}
		memset (buffer, 0, sizeof (buffer));
		putpwd (count, group, space);
		putc ('\n', stdout);
		device++;
	}
	return;
}

#endif



