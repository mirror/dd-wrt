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
 *   signed evse_cm_atten_char (struct session * session, struct channel * channel, struct message * message);
 *
 *   slac.h
 *
 *   EVSE-HLE sends average msound values to the PEV-HLE for a match
 *   decision;
 *
 *--------------------------------------------------------------------*/

#ifndef EVSE_CM_ATTEN_CHAR_SOURCE
#define EVSE_CM_ATTEN_CHAR_SOURCE

#include <string.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../iso15118/slac.h"

signed evse_cm_atten_char (struct session * session, struct channel * channel, struct message * message) 

{ 
	struct cm_atten_char_indicate * indicate = (struct cm_atten_char_indicate *) (message); 
	struct cm_atten_char_response * response = (struct cm_atten_char_response *) (message); 
	slac_debug (session, 0, __func__, "--> CM_ATTEN_CHAR.IND"); 
	memset (message, 0, sizeof (* message)); 
	EthernetHeader (& indicate->ethernet, session->PEV_MAC, channel->host, channel->type); 
	HomePlugHeader1 (& indicate->homeplug, HOMEPLUG_MMV, (CM_ATTEN_CHAR | MMTYPE_IND)); 
	indicate->APPLICATION_TYPE = session->APPLICATION_TYPE; 
	indicate->SECURITY_TYPE = session->SECURITY_TYPE; 
	memcpy (indicate->ACVarField.SOURCE_ADDRESS, session->PEV_MAC, sizeof (indicate->ACVarField.SOURCE_ADDRESS)); 
	memcpy (indicate->ACVarField.RunID, session->RunID, sizeof (indicate->ACVarField.RunID)); 
	memset (indicate->ACVarField.SOURCE_ID, 0, sizeof (indicate->ACVarField.SOURCE_ID)); 
	memset (indicate->ACVarField.RESP_ID, 0, sizeof (indicate->ACVarField.RESP_ID)); 
	indicate->ACVarField.NUM_SOUNDS = session->sounds; 
	indicate->ACVarField.ATTEN_PROFILE.NumGroups = session->NumGroups; 
	memcpy (indicate->ACVarField.ATTEN_PROFILE.AAG, session->AAG, session->NumGroups); 
	if (sendmessage (channel, message, sizeof (* indicate)) <= 0) 
	{ 
		return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
	} 
	if (readmessage (channel, message, HOMEPLUG_MMV, (CM_ATTEN_CHAR | MMTYPE_RSP)) > 0) 
	{ 
		if (! memcmp (session->RunID, response->ACVarField.RunID, sizeof (session->RunID))) 
		{ 
			slac_debug (session, 0, __func__, "<-- CM_ATTEN_CHAR.RSP"); 

#if SLAC_DEBUG

			if (_anyset (session->flags, SLAC_VERBOSE)) 
			{ 
				char string [256]; 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.RSP.APPLICATION_TYPE %d", response->APPLICATION_TYPE); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.RSP.SECURITY_TYPE %d", response->SECURITY_TYPE); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.RSP.ACVarfield.SOURCE_ADDRESS %s", HEXSTRING (string, response->ACVarField.SOURCE_ADDRESS)); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.RSP.ACVarFIeld.RunID %s", HEXSTRING (string, response->ACVarField.RunID)); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.RSP.ACVarField.SOURCE_ID %s", HEXSTRING (string, response->ACVarField.SOURCE_ID)); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.RSP.ACVarField.RESP_ID %s", HEXSTRING (string, response->ACVarField.RESP_ID)); 
				slac_debug (session, 0, __func__, "CM_ATTEN_CHAR.RSP.ACVarField.Result %d", response->ACVarField.Result); 
			} 

#endif

			return (0); 
		} 
	} 
	return (slac_debug (session, session->exit, __func__, "<-- CM_ATTEN_CHAR.RSP ?")); 
} 

#endif



