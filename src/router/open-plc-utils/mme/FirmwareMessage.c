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
 *   signed FirmwareMessage (void const * memory);
 *
 *   mme.h
 *
 *   intercept and print Qualcomm Atheros vendor-specific VS_ARPC
 *   messages on stdout; this message type is used for diagnostic
 *   reporting and should not appear in released firmware;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef FIRMWAREMESSAGE_SOURCE
#define FIRMWAREMESSAGE_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../plc/plc.h"
#include "../tools/memory.h"
#include "../tools/endian.h"
#include "../mme/mme.h"

signed FirmwareMessage (void const * memory)

{
	const struct message * message = (const struct message *)(memory);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	static struct qualcomm_hdr header_arpc =
	{
		0,
		0,
		{
			0x00,
			0xB0,
			0x52
		}
	};
	struct __packed vs_arpc_indicate
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint16_t RDATALENGTH;
		uint8_t RDATAOFFSET;
		uint8_t RDATA [1];
	}
	* indicate = (struct vs_arpc_indicate *)(message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	header_arpc.MMTYPE = HTOLE16 (VS_ARPC | MMTYPE_IND);
	if (!memcmp (&indicate->qualcomm, &header_arpc, sizeof (header_arpc)))
	{
		ARPCPrint (stderr, &indicate->RDATA [indicate->RDATAOFFSET], LE16TOH (indicate->RDATALENGTH) - indicate->RDATAOFFSET);
		return (-1);
	}
	return (0);
}


#endif

