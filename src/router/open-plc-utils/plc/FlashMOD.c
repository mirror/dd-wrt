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
 *   signed FlashMOD (struct channel * channel, uint8_t module);
 *
 *   plc.h
 *
 *   commit downloaded firmware and/or parameters to NVRAM using a
 *   VS_MOD_NVM message; flash-less devices will attempt to upload
 *   to the local host since they have no NVRAM; the host must be
 *   ready to receive the module;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *      Matthieu Poullet <m.poullet@avm.de>
 *
 *--------------------------------------------------------------------*/

#ifndef FLASHMOD_SOURCE
#define FLASHMOD_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../plc/plc.h"
#include "../tools/error.h"
#include "../tools/memory.h"

signed FlashMOD (struct channel * channel, uint8_t module)

{
	struct message message;

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_mod_nvm_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MODULEID;
	}
	* request = (struct vs_mod_nvm_request *) (&message);
	struct __packed vs_mod_nvm_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t MODULEID;
	}
	* confirm = (struct vs_mod_nvm_confirm *) (&message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	ssize_t packetsize;
	memset (&message, 0, sizeof (message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_MOD_NVM | MMTYPE_REQ));
	request->MODULEID = module;
	if (sendpacket (channel, &message, (ETHER_MIN_LEN - ETHER_CRC_LEN)) <= 0)
	{
		error (1, errno, CHANNEL_CANTSEND);
	}
	if ((packetsize = readpacket (channel, &message, sizeof (message))) <= 0)
	{
		error (1, errno, CHANNEL_CANTREAD);
	}
	if (UnwantedMessage (&message, packetsize, 0, (VS_MOD_NVM | MMTYPE_CNF)))
	{
		error (1, ECANCELED, PLC_BADFRAME);
	}
	if (confirm->MSTATUS)
	{
		error (1, ECANCELED, PLC_WONTDOIT);
	}
	return (0);
}


#endif

