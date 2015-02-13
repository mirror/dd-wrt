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
 *   void MMEPeek (void const * memory, size_t length, FILE *fp);
 *
 *   mme.h
 *
 *   print a HomePlug AV frame header on stdout in human readable
 *   format;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef MMPEEK_SOURCE
#define MMPEEK_SOURCE

#include <stdio.h>
#include <stdint.h>

#include "../tools/memory.h"
#include "../mme/mme.h"

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6 /* normally defined in ethernet.h or if_ether.h */
#endif

void MMEPeek (void const * memory, size_t extent, FILE *fp)

{
	char address [ETHER_ADDR_LEN * 3];
	struct message * message = (struct message *)(memory);
	fprintf (fp, "ODA=%s ", hexstring (address, sizeof (address), message->ethernet.ODA, sizeof (message->ethernet.ODA)));
	fprintf (fp, "OSA=%s ", hexstring (address, sizeof (address), message->ethernet.OSA, sizeof (message->ethernet.OSA)));
	fprintf (fp, "MTYPE=%04X ", ntohs (message->ethernet.MTYPE));
	if (ntohs (message->ethernet.MTYPE) == ETH_P_HPAV)
	{
		struct homeplug_hdr * homeplug = (struct homeplug_hdr *)(&message->content);
		uint16_t mmtype = LE16TOH (homeplug->MMTYPE);
		if (mmtype < MMTYPE_VS)
		{
			if (homeplug->MMV == 0x00)
			{
				struct homeplug_hdr * homeplug = (struct homeplug_hdr *)(&message->content);
				mmtype = LE16TOH (homeplug->MMTYPE);
				fprintf (fp, "MMV=%02X ", homeplug->MMV);
				fprintf (fp, "MMTYPE=%04X ", mmtype);
				fprintf (fp, "%s.%s\n", MMEName (mmtype), MMEMode (mmtype));
				return;
			}
			if (homeplug->MMV == 0x01)
			{
				struct homeplug_fmi * homeplug = (struct homeplug_fmi *)(&message->content);
				mmtype = LE16TOH (homeplug->MMTYPE);
				fprintf (fp, "MMV=%02X ", homeplug->MMV);
				fprintf (fp, "MMTYPE=%04X ", mmtype);
				fprintf (fp, "FMID=%02X ", homeplug->FMID);
				fprintf (fp, "FMSN=%02X ", homeplug->FMSN);
				fprintf (fp, "%s.%s\n", MMEName (mmtype), MMEMode (mmtype));
				return;
			}
		}
		if (mmtype < MMTYPE_XX)
		{
			if (homeplug->MMV == 0x00)
			{
				struct qualcomm_hdr * qualcomm = (struct qualcomm_hdr *)(&message->content);
				mmtype = LE16TOH (qualcomm->MMTYPE);
				fprintf (fp, "MMV=%02X ", qualcomm->MMV);
				fprintf (fp, "MMTYPE=%04X ", mmtype);
				fprintf (fp, "OUI=%s ", hexstring (address, sizeof (address), qualcomm->OUI, sizeof (qualcomm->OUI)));
				fprintf (fp, "%s.%s\n", MMEName (mmtype), MMEMode (mmtype));
				return;
			}
			if (homeplug->MMV == 0x01)
			{
				struct qualcomm_fmi * qualcomm = (struct qualcomm_fmi *)(&message->content);
				mmtype = LE16TOH (qualcomm->MMTYPE);
				fprintf (fp, "MMV=%02X ", qualcomm->MMV);
				fprintf (fp, "MMTYPE=%04X ", mmtype);
				fprintf (fp, "FMID=%02X ", qualcomm->FMID);
				fprintf (fp, "FMSN=%02X ", qualcomm->FMSN);
				fprintf (fp, "OUI=%s ", hexstring (address, sizeof (address), qualcomm->OUI, sizeof (qualcomm->OUI)));
				fprintf (fp, "%s.%s\n", MMEName (mmtype), MMEMode (mmtype));
				return;
			}
		}
	}
	fprintf (fp, "UNKNOWN_MESSAGE_TYPE\n");
	return;
}


#endif

