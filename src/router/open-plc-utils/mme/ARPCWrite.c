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
 *   void ARPCWrite (signed fd, void const * memory, size_t extent);
 *
 *   mme.h
 *
 *   write an unformatted VS_ARPC payload on the specified output
 *   stream; this implementation is generic; memory is the start
 *   address of the message data (&RDATA [RDATAOFFSET]) and extent
 *   is the data length (RDATALENGTH); the call might look like
 *   this ...
 *
 *   ARPCWrite (fd, &ARPC->RDATA [ARPC->RDATAOFFSET], LE16TOH (ARPC->RDATALENGTH - ARPC->RDATAOFFSET));
 *
 *   ... where LE16TOH () performs 16-bit host endian conversion;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef ARPCWRITE_SOURCE
#define ARPCWRITE_SOURCE

#include <stdio.h>

#include "../tools/types.h"
#include "../tools/endian.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../mme/mme.h"

void ARPCWrite (signed fd, void const * memory, size_t extent)

{

#if 0

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

#endif
    uint16_t length = (uint16_t)(extent);
	if (isatty (fd))
	{
		hexwrite (fd, &length, sizeof (length));
		hexwrite (fd, memory, length);

#if defined (_WIN32)

		write (fd, "\r", sizeof (char));

#endif

		write (fd, "\n", sizeof (char));
	}
	else
	{
		write (fd, &length, sizeof (length));
		write (fd, memory, length);
	}
	return;
}


#endif

