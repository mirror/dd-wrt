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
 *   void  MACPasswords (unsigned vendor, unsigned device, unsigned number, unsigned count, unsigned group, char space, flag_t flags);
 *
 *   keys.h
 *
 *   print a range of device address/password pairs on stdout; print
 *   an optional usage flag in the first column for PTS compatability;
 *
 *   vendor is the 24-bit OUI expressed as an integer; device is the
 *   24-bit starting unit address expressed as an integer; number is
 *   the number of address/password pairs to generate; count is the
 *   number of letters in the password excluding delimiters;
 *
 *   passwords consists of letters arranged in groups separated by
 *   spaces; count is the number of letters; group is the number of
 *   letters in each group; space is the character that separates
 *   each group;
 *
 *   vendor is used to seed the random number generator and create
 *   a character set having the 256 random upper case letters used
 *   for all vendor passwords; most letters will appear more than
 *   once in the character set;
 *
 *   device is used to seed the random number generator and select
 *   count random letters from the character set until the password
 *   has been constructed;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef MACPASSWORDS_SOURCE
#define MACPASSWORDS_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include "../tools/types.h"
#include "../tools/flags.h"
#include "../key/keys.h"

static uint64_t MACSeed = 0;
static uint64_t MACSRand (uint64_t seed)

{
	uint64_t temp = MACSeed;
	MACSeed = seed;
	return (temp);
}

static unsigned MACRand ()

{
	MACSeed *= 0x41C64E6D;
	MACSeed += 0x00003029;
	return ((unsigned)((MACSeed >> 0x10) & 0x7FFFFFFF));
}

/*====================================================================*
 *
 *   void MACPassword (unsigned device, char const charset [], unsigned limit, unsigned alpha, unsigned bunch, char space);
 *
 *   keys.h
 *
 *   device is used to seed the random number generator and select
 *   count random letters from the character set until the password
 *   has been constructed; alpha is the total number of letters in
 *   the password, excluding delimiters; bunch is a grouping factor
 *   for letters; space is the character used to separate groups;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

void MACPassword (unsigned device, char const charset [], unsigned limit, unsigned alpha, unsigned bunch, char space)

{
	MACSRand (device);
	while (alpha--)
	{
		unsigned index = MACRand () % limit;
		putc (charset [index & limit], stdout);
		if ((alpha) && (bunch) && !(alpha % bunch))
		{
			putc (space, stdout);
		}
	}
	return;
}

/*====================================================================*
 *
 *   void  MACPasswords (unsigned vendor, unsigned device, unsigned number, unsigned count, unsigned group, char space, flag_t flags);
 *
 *   keys.h
 *
 *   print a range of device address/password pairs on stdout; print
 *   an optional usage flag in the first column for PTS compatability;
 *
 *   vendor is the 24-bit OUI expressed as an integer; device is the
 *   24-bit starting unit address expressed as an integer; number is
 *   the number of address/password pairs to generate; count is the
 *   number of letters in the password excluding delimiters;
 *
 *   passwords consists of letters arranged in groups separated by
 *   spaces; count is the number of letters; group is the number of
 *   letters in each group; space is the character that separates
 *   each group;
 *
 *   vendor is used to seed the random number generator and create
 *   a character set having the 256 random upper case letters used
 *   for all vendor passwords; most letters will appear more than
 *   once in the character set;
 *
 *   device is used to seed the random number generator and select
 *   count random letters from the character set until the password
 *   has been constructed;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

void MACPasswords (unsigned vendor, unsigned device, unsigned count, unsigned alpha, unsigned bunch, char space, flag_t flags)

{
	char charset [UCHAR_MAX];
	unsigned offset = 0;
	if (vendor >> 24)
	{
		return;
	}
	if (device >> 24)
	{
		return;
	}
	if (count >> 24)
	{
		return;
	}
	MACSRand (vendor);
	while (offset < sizeof (charset))
	{
		unsigned c = MACRand () % (SCHAR_MAX + 1);
		if (isupper (c))
		{
			charset [offset++] = c;
		}
	}
	while (count--)
	{
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
		MACPassword (device, charset, sizeof (charset), alpha, bunch, space);
		putc ('\n', stdout);
		device++;
	}
	return;
}


#endif

