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
 *   uint16_t psread (uint16_t values [], uint16_t limit, FILE * fp);
 *
 *   pib.h
 *
 *   read a prescaler file and populate a scaler array;
 *
 *
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "../tools/chars.h"
#include "../tools/error.h"
#include "../tools/number.h"
#include "../pib/pib.h"

uint16_t psread (uint16_t values [], uint16_t limit, FILE * fp)

{
	uint16_t index = 0;
	uint16_t count = 0;
	uint32_t value = 0;
	signed c;
	while ((c = getc (fp)) != EOF)
	{
		if (isspace (c))
		{
			continue;
		}
		if ((c == '#') || (c == ';'))
		{
			do
			{
				c = getc (fp);
			}
			while ((c != '\n') && (c != EOF));
			continue;
		}
		index = 0;
		while (isdigit (c))
		{
			index *= 10;
			index += c - '0';
			c = getc (fp);
		}
		if (index != count)
		{
			error (1, ECANCELED, "Carrier %d out of order", index);
		}
		if (index >= limit)
		{
			error (1, EOVERFLOW, "Too many prescalers");
		}
		while (isblank (c))
		{
			c = getc (fp);
		}
		value = 0;
		while (isxdigit (c))
		{
			value *= 16;
			value += todigit (c);
			c = getc (fp);
		}
		values [index] = value;
		while ((c != '\n') && (c != EOF))
		{
			c = getc (fp);
		};
		count++;
	}
	return (count);
}

