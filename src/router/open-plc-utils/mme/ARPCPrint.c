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
 *   void ARPCPrint (FILE * fp, void const * memory, size_t extent);
 *
 *   mme.h
 *
 *   print formatted VS_ARPC payload on the specified output stream;
 *   this implementation is generic; memory is the start address of
 *   the message data (&RDATA [RDATAOFFSET]) and extent is the data
 *   length (RDATALENGTH); the call might look like this ...
 *
 *   ARPCPrint (fp, &ARPC->RDATA [ARPC->RDATAOFFSET], LE16TOH (ARPC->RDATALENGTH) - ARPC->RDATAOFFSET);
 *
 *   ... where LE16TOH () performs 16-bit host endian conversion;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *	Florian Fainelli <f.fainelli@gmail.com>
 *
 *--------------------------------------------------------------------*/

#ifndef ARPCPRINT_SOURCE
#define ARPCPRINT_SOURCE

#include <stdio.h>
#include <stddef.h>

#include "../tools/types.h"
#include "../tools/endian.h"
#include "../tools/memory.h"
#include "../mme/mme.h"

void ARPCPrint (FILE * fp, void const * memory, size_t extent)

{

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_arpc_data
	{
		uint32_t BYPASS;
		uint16_t ARPCID;
		uint16_t DATALENGTH;
		uint8_t DATAOFFSET;
		uint8_t RESERVED [3];
		uint16_t ARGOFFSET;
		uint16_t STROFFSET;
		uint16_t ARGLENGTH;
		uint16_t STRLENGTH;
		uint8_t LIST [1];
	}
	* data = (struct vs_arpc_data *)(memory);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	uint32_t * argp = (uint32_t *)(&data->LIST [LE16TOH (data->ARGOFFSET)]);
	uint16_t argc = LE16TOH (data->ARGLENGTH) >> 2;
	while (argc--)
	{
		*argp = LE32TOH (*argp);
		argp++;
	}

#if defined (__UCLIBC__) 

/*
 *	This is a simple fix to work around uclibc implementation issue with variable arugment lists; We are
 *	testing a better one but most PLC users will not have access to firmware that exercises this module;
 */

	argp = (uint32_t *)(&data->LIST [LE16TOH (data->ARGOFFSET)]);
	fprintf (fp, (char *)(&data->LIST [LE16TOH (data->STROFFSET)]), argp [0], argp [1], argp [2], argp [3], argp [4], argp [5], argp [6], argp [7], argp [8], argp [9], argp [10], argp [11], argp [12], argp [13], argp [14], argp [15], argp [16], argp [17], argp [18], argp [19], argp [20], argp [21], argp [22], argp [23], argp [24], argp [25], argp [26], argp [27], argp [28], argp [29], argp [30], argp [31], argp [32], argp [33], argp [34], argp [35], argp [36], argp [37], argp [38], argp [39]);

#else

	vfprintf (fp, (char *)(&data->LIST [LE16TOH (data->STROFFSET)]), (void *)(&data->LIST [LE16TOH (data->ARGOFFSET)]));

#endif

	fprintf (fp, "\n");
	fflush (fp);
	return;
}


#endif

