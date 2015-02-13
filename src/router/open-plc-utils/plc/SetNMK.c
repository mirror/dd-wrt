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
 *   signed SetNMK (struct plc * plc);
 *
 *   plc.h
 *
 *   set NMK on a local or remote device using a VS_SET_KEY message;
 *
 *   using this message to set the NMK on a remote device requires
 *   both the remote device address (RDA) and the remote device
 *   access key (DAK);
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef SETNMK_SOURCE
#define SETNMK_SOURCE

#include <string.h>

#include "../plc/plc.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../tools/memory.h"
#include "../key/HPAVKey.h"

signed SetNMK (struct plc * plc)

{
	extern const byte localcast [ETHER_ADDR_LEN];
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_set_key_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t EKS;
		uint8_t NMK [HPAVKEY_NMK_LEN];
		uint8_t PEKS;
		uint8_t RDA [ETHER_ADDR_LEN];
		uint8_t DAK [HPAVKEY_DAK_LEN];
	}
	* request = (struct vs_set_key_request *) (message);
	struct __packed vs_set_key_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
	}
	* confirm = (struct vs_set_key_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, localcast, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_SET_KEY | MMTYPE_REQ));
	plc->packetsize = sizeof (struct vs_set_key_request);
	request->EKS = 0x01;
	memcpy (request->NMK, plc->NMK, sizeof (request->NMK));
	if (_anyset (plc->flags, PLC_SETREMOTEKEY))
	{
		Request (plc, "Set Remote Network Membership Key");
		memcpy (request->RDA, plc->RDA, sizeof (request->RDA));
		memcpy (request->DAK, plc->DAK, sizeof (request->DAK));
		request->PEKS = 0x00;
	}
	else
	{
		Request (plc, "Set Local Network Membership Key");
		memset (request->RDA, 0, sizeof (request->RDA));
		memset (request->DAK, 0, sizeof (request->DAK));
		request->PEKS = 0x0F;
	}
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	if (ReadMME (plc, 0, (VS_SET_KEY | MMTYPE_CNF)) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
		return (-1);
	}
	if (confirm->MSTATUS)
	{
		Failure (plc, PLC_WONTDOIT);
		return (-1);
	}
	Confirm (plc, "Setting ...");
	return (0);
}


#endif

