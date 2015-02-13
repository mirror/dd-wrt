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
 *   signed evse_cm_slac_match (struct session * session, struct channel * channel, struct message * message);
 *
 *   slac.h
 *  
 *   receive CM_SLAC_MATCH.REQ and send CM_SLAC_MATCH.CNF containing
 *   random NMK/NID that EVSE-HLE will use to configure EVSE-PLC for
 *   charging; 
 *
 *   users must develop their own means of generating random NMK/NID
 *   pairs because it is not part of the standard; this example does
 *   not attempt to randomize NMK/NID;
 *  
 *--------------------------------------------------------------------*/

#ifndef EVSE_CM_SLAC_MATCH_SOURCE
#define EVSE_CM_SLAC_MATCH_SOURCE

#include <stdio.h>
#include <string.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../slac/slac.h"

signed evse_cm_slac_match (struct session * session, struct channel * channel, struct message * message) 

{ 
	struct cm_slac_match_request * request = (struct cm_slac_match_request *) (message); 
	struct cm_slac_match_confirm * confirm = (struct cm_slac_match_confirm *) (message); 
	while (readmessage (channel, message, HOMEPLUG_MMV, (CM_SLAC_MATCH | MMTYPE_REQ)) > 0) 
	{ 
		if (! memcmp (session->RunID, request->MatchVarField.RunID, sizeof (session->RunID))) 
		{ 
			slac_debug (session, 0, __func__, "<-- CM_SLAC_MATCH.REQ"); 
			memcpy (session->PEV_ID, request->MatchVarField.PEV_ID, sizeof (session->PEV_ID)); 
			memcpy (session->PEV_MAC, request->MatchVarField.PEV_MAC, sizeof (session->PEV_MAC)); 
			memcpy (session->RunID, request->MatchVarField.RunID, sizeof (session->RunID)); 

#if SLAC_DEBUG

			if (_anyset (session->flags, SLAC_VERBOSE)) 
			{ 
				char string [256]; 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.REQ.APPLICATION_TYPE %d", request->APPLICATION_TYPE); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.REQ.SECURITY_TYPE %d", request->SECURITY_TYPE); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.REQ.MVFLength %d", LE16TOH (request->MVFLength)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.REQ.PEV_ID %s", HEXSTRING (string, request->MatchVarField.PEV_ID)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.REQ.PEV_MAC %s", HEXSTRING (string, request->MatchVarField.PEV_MAC)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.REQ.EVSE_ID %s", HEXSTRING (string, request->MatchVarField.EVSE_ID)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.REQ.EVSE_MAC %s", HEXSTRING (string, request->MatchVarField.EVSE_MAC)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.REQ.RunID %s", HEXSTRING (string, request->MatchVarField.RunID)); 
			} 

#endif

			slac_debug (session, 0, __func__, "--> CM_SLAC_MATCH.CNF"); 
			memset (message, 0, sizeof (* message)); 
			EthernetHeader (& confirm->ethernet, session->PEV_MAC, channel->host, channel->type); 
			HomePlugHeader1 (& confirm->homeplug, HOMEPLUG_MMV, (CM_SLAC_MATCH | MMTYPE_CNF)); 
			confirm->APPLICATION_TYPE = session->APPLICATION_TYPE; 
			confirm->SECURITY_TYPE = session->SECURITY_TYPE; 
			confirm->MVFLength = HTOLE16 (sizeof (confirm->MatchVarField)); 
			memcpy (confirm->MatchVarField.PEV_ID, session->PEV_ID, sizeof (confirm->MatchVarField.PEV_ID)); 
			memcpy (confirm->MatchVarField.PEV_MAC, session->PEV_MAC, sizeof (confirm->MatchVarField.PEV_MAC)); 
			memcpy (confirm->MatchVarField.EVSE_ID, session->EVSE_ID, sizeof (confirm->MatchVarField.EVSE_ID)); 
			memcpy (confirm->MatchVarField.EVSE_MAC, session->EVSE_MAC, sizeof (confirm->MatchVarField.EVSE_MAC)); 
			memcpy (confirm->MatchVarField.RunID, session->RunID, sizeof (confirm->MatchVarField.RunID)); 
			memcpy (confirm->MatchVarField.NID, session->NID, sizeof (confirm->MatchVarField.NID)); 
			memcpy (confirm->MatchVarField.NMK, session->NMK, sizeof (confirm->MatchVarField.NMK)); 
			if (sendmessage (channel, message, sizeof (* confirm)) <= 0) 
			{ 
				return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
			} 
			return (0); 
		} 
	} 
	return (slac_debug (session, session->exit, __func__, "<-- CM_SLAC_MATCH.REQ ?")); 
} 

#endif



