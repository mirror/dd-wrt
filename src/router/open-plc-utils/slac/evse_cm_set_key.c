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
 *   signed evse_cm_set_key (struct session * session, struct channel * channel, struct message * message);
 *
 *   slac.h
 *
 *   PEV-HLE sets the NMK and NID on PEV-PLC using CM_SET_KEY.REQ; 
 *   the NMK and NID must match those provided by EVSE-HLE using 
 *   CM_SLAC_MATCH.CNF;
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef EVSE_CM_SET_KEY_SOURCE
#define EVSE_CM_SET_KEY_SOURCE

#include <string.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../mme/qualcomm.h"
#include "../mme/homeplug.h"
#include "../slac/slac.h"

signed evse_cm_set_key (struct session * session, struct channel * channel, struct message * message) 

{ 

#ifndef __GNUC__
#pragma pack(push,1)
#endif

	struct __packed cm_set_key_request 
	{ 
		struct ethernet_hdr ethernet; 
		struct homeplug_fmi homeplug; 
		uint8_t KEYTYPE; 
		uint32_t MYNOUNCE; 
		uint32_t YOURNOUNCE; 
		uint8_t PID; 
		uint16_t PRN; 
		uint8_t PMN; 
		uint8_t CCOCAP; 
		uint8_t NID [SLAC_NID_LEN]; 
		uint8_t NEWEKS; 
		uint8_t NEWKEY [SLAC_NMK_LEN]; 
		uint8_t RSVD [3]; 
	} 
	* request = (struct cm_set_key_request *) (message); 
	struct __packed cm_set_key_confirm 
	{ 
		struct ethernet_hdr ethernet; 
		struct homeplug_fmi homeplug; 
		uint8_t RESULT; 
		uint32_t MYNOUNCE; 
		uint32_t YOURNOUNCE; 
		uint8_t PID; 
		uint16_t PRN; 
		uint8_t PMN; 
		uint8_t CCOCAP; 
		uint8_t RSVD [27]; 
	} 
	* confirm = (struct cm_set_key_confirm *) (message); 

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message)); 
	slac_debug (session, 0, __func__, "--> CM_SET_KEY.REQ"); 
	EthernetHeader (& request->ethernet, channel->peer, channel->host, channel->type); 
	HomePlugHeader1 (& request->homeplug, HOMEPLUG_MMV, (CM_SET_KEY | MMTYPE_REQ)); 
	request->KEYTYPE = SLAC_CM_SETKEY_KEYTYPE; 
	memset (& request->MYNOUNCE, 0xAA, sizeof (request->MYNOUNCE)); 
	memset (& request->YOURNOUNCE, 0x00, sizeof (request->YOURNOUNCE)); 
	request->PID = SLAC_CM_SETKEY_PID; 
	request->PRN = HTOLE16 (SLAC_CM_SETKEY_PRN); 
	request->PMN = SLAC_CM_SETKEY_PMN; 
	request->CCOCAP = SLAC_CM_SETKEY_CCO; 
	memcpy (request->NID, session->NID, sizeof (request->NID)); 
	request->NEWEKS = SLAC_CM_SETKEY_EKS; 
	memcpy (request->NEWKEY, session->NMK, sizeof (request->NEWKEY)); 

#if SLAC_DEBUG

	if (_anyset (session->flags, SLAC_VERBOSE)) 
	{ 
		char string [1024]; 
		slac_debug (session, 0, __func__, "CM_SET_KEY.KEYTYPE %d", request->KEYTYPE); 
		slac_debug (session, 0, __func__, "CM_SET_KEY.MYNOUNCE %s", hexstring (string, sizeof (string), & request->MYNOUNCE, sizeof (request->MYNOUNCE))); 
		slac_debug (session, 0, __func__, "CM_SET_KEY.YOURNOUNCE %s", hexstring (string, sizeof (string), & request->YOURNOUNCE, sizeof (request->MYNOUNCE))); 
		slac_debug (session, 0, __func__, "CM_SET_KEY.PID %d", request->PID); 
		slac_debug (session, 0, __func__, "CM_SET_KEY.PRN %d", LE32TOH (request->PRN)); 
		slac_debug (session, 0, __func__, "CM_SET_KEY.PMN %d", request->PMN); 
		slac_debug (session, 0, __func__, "CM_SET_KEY.CCoCAP %d", request->CCOCAP); 
		slac_debug (session, 0, __func__, "CM_SET_KEY.NID %s", HEXSTRING (string, request->NID)); 
		slac_debug (session, 0, __func__, "CM_SET_KEY.NEWEKS %d", request->NEWEKS); 
		slac_debug (session, 0, __func__, "CM_SET_KEY.NEWKEY %s", HEXSTRING (string, request->NEWKEY)); 
	} 

#endif

	if (sendpacket (channel, request, sizeof (* request)) <= 0) 
	{ 
		return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
	} 
	while (readpacket (channel, confirm, sizeof (* confirm)) > 0) 
	{ 
		if (ntohs (confirm->ethernet.MTYPE) != ETH_P_HPAV) 
		{ 
			slac_debug (session, session->exit, __func__, "Ignore MTYPE 0x%04X", htons (confirm->ethernet.MTYPE)); 
			continue; 
		} 
		if (confirm->homeplug.MMV != HOMEPLUG_MMV) 
		{ 
			slac_debug (session, session->exit, __func__, "Ignore MMV 0x%02X", confirm->homeplug.MMV); 
			continue; 
		} 
		if (LE32TOH (confirm->homeplug.MMTYPE) != (CM_SET_KEY | MMTYPE_CNF)) 
		{ 
			slac_debug (session, session->exit, __func__, "Ignore MMTYPE 0x%04X", LE32TOH (confirm->homeplug.MMTYPE)); 
			continue; 
		} 
		slac_debug (session, 0, __func__, "<-- CM_SET_KEY.CNF"); 
		if (! confirm->RESULT) 
		{ 
			return (slac_debug (session, session->exit, __func__, "Device refused request")); 
		} 

#if SLAC_DEBUG

		if (_anyset (session->flags, SLAC_VERBOSE)) 
		{ 
			char string [1024]; 
			slac_debug (session, 0, __func__, "CM_SET_KEY.RESULT %d", confirm->RESULT); 
			slac_debug (session, 0, __func__, "CM_SET_KEY.MYNOUNCE %s", hexstring (string, sizeof (string), & confirm->MYNOUNCE, sizeof (confirm->MYNOUNCE))); 
			slac_debug (session, 0, __func__, "CM_SET_KEY.YOURNOUNCE %s", hexstring (string, sizeof (string), & confirm->YOURNOUNCE, sizeof (confirm->MYNOUNCE))); 
			slac_debug (session, 0, __func__, "CM_SET_KEY.PID %d", confirm->PID); 
			slac_debug (session, 0, __func__, "CM_SET_KEY.PRN %d", LE32TOH (confirm->PRN)); 
			slac_debug (session, 0, __func__, "CM_SET_KEY.PMN %d", confirm->PMN); 
			slac_debug (session, 0, __func__, "CM_SET_KEY.CCoCAP %d", confirm->CCOCAP); 
		} 

#endif

		return (0); 
	} 
	return (slac_debug (session, session->exit, __func__, "<-- CM_SET_KEY.REQ ?")); 
} 

#endif



