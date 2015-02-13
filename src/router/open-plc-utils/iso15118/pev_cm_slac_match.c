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
 *   signed pev_cm_slac_match (struct session * session, struct channel * channel, struct message * message);
 *
 *   slac.h
 *
 *   send CM_SLAC_MATCH.REQ and receive CM_SLAC_MATCH.CNF; store NMK
 *   and NID in the session variable so that PEV-HLE can configure 
 *   PEV-PLC to form a network with EVSE-PLC;
 *
 *--------------------------------------------------------------------*/

#ifndef PEV_CM_SLAC_MATCH_SOURCE
#define PEV_CM_SLAC_MATCH_SOURCE

#include <string.h>
#include <errno.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../iso15118/slac.h"

signed pev_cm_slac_match (struct session * session, struct channel * channel, struct message * message) 

{ 
	struct cm_slac_match_request * request = (struct cm_slac_match_request *) (message); 
	struct cm_slac_match_confirm * confirm = (struct cm_slac_match_confirm *) (message); 
	slac_debug (session, 0, __func__, "--> CM_SLAC_MATCH.REQ"); 
	memset (message, 0, sizeof (* message)); 
	EthernetHeader (& request->ethernet, session->EVSE_MAC, channel->host, channel->type); 
	HomePlugHeader1 (& request->homeplug, HOMEPLUG_MMV, (CM_SLAC_MATCH | MMTYPE_REQ)); 
	request->APPLICATION_TYPE = session->APPLICATION_TYPE; 
	request->SECURITY_TYPE = session->SECURITY_TYPE; 
	request->MVFLength = HTOLE16 (sizeof (request->MatchVarField)); 
	memcpy (request->MatchVarField.PEV_ID, session->PEV_ID, sizeof (request->MatchVarField.PEV_ID)); 
	memcpy (request->MatchVarField.PEV_MAC, session->PEV_MAC, sizeof (request->MatchVarField.PEV_MAC)); 
	memcpy (request->MatchVarField.RunID, session->RunID, sizeof (request->MatchVarField.RunID)); 
	if (sendmessage (channel, message, sizeof (* request)) <= 0) 
	{ 
		return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
	} 
	if (readmessage (channel, message, HOMEPLUG_MMV, (CM_SLAC_MATCH | MMTYPE_CNF)) > 0) 
	{ 
		if (! memcmp (session->RunID, confirm->MatchVarField.RunID, sizeof (session->RunID))) 
		{ 
			slac_debug (session, 0, __func__, "<-- CM_SLAC_MATCH.CNF"); 

#if SLAC_DEBUG

			if (_anyset (session->flags, SLAC_VERBOSE)) 
			{ 
				char string [256]; 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.APPLICATION_TYPE %d", confirm->APPLICATION_TYPE); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.SECURITY_TYPE %d", confirm->SECURITY_TYPE); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.MVFLength %d", LE16TOH (confirm->MVFLength)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.MatchVarField.PEV_ID %s", HEXSTRING (string, confirm->MatchVarField.PEV_ID)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.MatchVarField.PEV_MAC %s", HEXSTRING (string, confirm->MatchVarField.PEV_MAC)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.MatchVarField.EVSE_ID %s", HEXSTRING (string, confirm->MatchVarField.EVSE_ID)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.MatchVarField.EVSE_MAC %s", HEXSTRING (string, confirm->MatchVarField.EVSE_MAC)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.MatchVarField.RunID %s", HEXSTRING (string, confirm->MatchVarField.RunID)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.MatchVarField.NID %s", HEXSTRING (string, confirm->MatchVarField.NID)); 
				slac_debug (session, 0, __func__, "CM_SLAC_MATCH.CNF.MatchVarField.NMK %s", HEXSTRING (string, confirm->MatchVarField.NMK)); 
			} 

#endif

			memcpy (session->EVSE_ID, confirm->MatchVarField.EVSE_ID, sizeof (session->EVSE_ID)); 
			memcpy (session->EVSE_MAC, confirm->MatchVarField.EVSE_MAC, sizeof (session->EVSE_MAC)); 
			memcpy (session->NMK, confirm->MatchVarField.NMK, sizeof (session->NMK)); 
			memcpy (session->NID, confirm->MatchVarField.NID, sizeof (session->NID)); 
			return (0); 
		} 
	} 
	return (slac_debug (session, session->exit, __func__, "<-- CM_SLAC_MATCH.CNF ?")); 
} 

#endif



