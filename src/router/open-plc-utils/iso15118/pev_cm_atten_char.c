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
 *   signed pev_cm_atten_char (struct session * session, struct channel * channel, struct message * message);
 *
 *   slac.h
 *
 *   PEV-HLE waits for CM_ATTEN_CHAR.IND from EVSE-HLE and responds
 *   to indicate the data was received in good order; this response
 *   does not mean that PEV-HLE will mate with EVSE-HLE;
 *
 *--------------------------------------------------------------------*/

#ifndef PEV_CM_ATTEN_CHAR_SOURCE
#define PEV_CM_ATTEN_CHAR_SOURCE

#include <string.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../iso15118/slac.h"

signed pev_cm_atten_char (struct session * session, struct channel * channel, struct message * message) 

{ 
	struct cm_atten_char_indicate * indicate = (struct cm_atten_char_indicate *) (message); 
	struct cm_atten_char_response * response = (struct cm_atten_char_response *) (message); 
	while (readmessage (channel, message, HOMEPLUG_MMV, (CM_ATTEN_CHAR | MMTYPE_IND)) > 0) 
	{ 
		if (! memcmp (session->RunID, indicate->ACVarField.RunID, sizeof (session->RunID))) 
		{ 
			slac_debug (session, 0, __func__, "<-- CM_ATTEN_CHAR.IND"); 
			memcpy (session->EVSE_MAC, indicate->ethernet.OSA, sizeof (session->EVSE_MAC)); 
			session->NUM_SOUNDS = indicate->ACVarField.NUM_SOUNDS; 
			session->NumGroups = indicate->ACVarField.ATTEN_PROFILE.NumGroups; 
			memcpy (session->AAG, indicate->ACVarField.ATTEN_PROFILE.AAG, indicate->ACVarField.ATTEN_PROFILE.NumGroups); 

#if SLAC_DEBUG

			if (_anyset (session->flags, SLAC_VERBOSE)) 
			{ 
				char string [256]; 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.IND.ACVarField.APPLICATION_TYPE %d", indicate->APPLICATION_TYPE); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.IND.ACVarField.SECURITY_TYPE %d", indicate->SECURITY_TYPE); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.IND.ACVarField.SOURCE_ADDRESS %s", HEXSTRING (string, indicate->ACVarField.SOURCE_ADDRESS)); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.IND.ACVarField.RunID %s", HEXSTRING (string, indicate->ACVarField.RunID)); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.IND.ACVarField.SOURCE_ID %s", HEXSTRING (string, indicate->ACVarField.SOURCE_ID)); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.IND.ACVarField.RESP_ID %s", HEXSTRING (string, indicate->ACVarField.RESP_ID)); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.IND.ACVarField.NUM_SOUNDS %d", indicate->ACVarField.NUM_SOUNDS); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.IND.ACVarField.ATTEN_PROFILE.NumGroups %d", indicate->ACVarField.ATTEN_PROFILE.NumGroups); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.IND.ACVarField.ATTEN_PROFILE.AAG %s", hexstring (string, sizeof (string), indicate->ACVarField.ATTEN_PROFILE.AAG, indicate->ACVarField.ATTEN_PROFILE.NumGroups)); 
			} 

#endif

			slac_debug (session, 0, __func__, "--> CM_ATTEN_CHAR.RSP"); 
			memset (message, 0, sizeof (* message)); 
			EthernetHeader (& response->ethernet, session->EVSE_MAC, channel->host, channel->type); 
			HomePlugHeader1 (& response->homeplug, HOMEPLUG_MMV, (CM_ATTEN_CHAR | MMTYPE_RSP)); 
			response->APPLICATION_TYPE = session->APPLICATION_TYPE; 
			response->SECURITY_TYPE = session->SECURITY_TYPE; 
			memcpy (response->ACVarField.SOURCE_ADDRESS, session->PEV_MAC, sizeof (response->ACVarField.SOURCE_ADDRESS)); 
			memcpy (response->ACVarField.RunID, session->RunID, sizeof (response->ACVarField.RunID)); 
			memset (response->ACVarField.SOURCE_ID, 0, sizeof (response->ACVarField.SOURCE_ID)); 
			memset (response->ACVarField.RESP_ID, 0, sizeof (response->ACVarField.RESP_ID)); 
			response->ACVarField.Result = 0; 
			if (sendmessage (channel, message, sizeof (* response)) <= 0) 
			{ 
				return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
			} 
			return (0); 
		} 
	} 
	return (slac_debug (session, session->exit, __func__, "<-- CM_ATTEN_CHAR.IND ?")); 
} 

#endif



