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
 *   signed ReadFMI (struct plc * plc, uint8_t MMV, uint16_t MMTYPE);
 *
 *   plc.h
 *
 *   read a fragmented message and return a pointer to a buffer that
 *   contains the concatenated message fragments; the buffer address
 *   is returned in plc->content; the calling function must free the
 *   buffer when done; buffer length is computed from the number of
 *   fragments returned in the FMI field of the first fragment;
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef READFMI_SOURCE
#define READFMI_SOURCE

#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../plc/plc.h"

signed ReadFMI (struct plc * plc, uint8_t MMV, uint16_t MMTYPE)

{
	if (ReadMME (plc, MMV, MMTYPE) > 0)
	{
		struct homeplug * homeplug = (struct homeplug *)(plc->message);
		unsigned count = ((homeplug->homeplug.FMID >> 0) & 0x0F);
		unsigned extra = ((homeplug->homeplug.FMID >> 4) & 0x0F);
		unsigned length = sizeof (* homeplug) + extra * sizeof (homeplug->content);
		if ((plc->content = malloc (length)))
		{
			signed offset = plc->packetsize;
			memcpy (plc->content, homeplug, offset);
			while (count < extra)
			{
				if (ReadMME (plc, MMV, MMTYPE) <= 0)
				{
					free (plc->content);
					plc->content = NULL;
					return (- 1);
				}
				count = ((homeplug->homeplug.FMID >> 0) & 0x0F);
				extra = ((homeplug->homeplug.FMID >> 4) & 0x0F);
				plc->packetsize -= sizeof (struct ethernet_hdr);
				plc->packetsize -= sizeof (struct homeplug_fmi);
				memcpy ((uint8_t *)(plc->content) +  offset, homeplug->content, plc->packetsize);
				offset += plc->packetsize;
			} 
			plc->packetsize = offset;
		}
		else
		{
			error (1, errno, "%s", __func__);
		}
	}
	return (plc->packetsize);
}

#endif


