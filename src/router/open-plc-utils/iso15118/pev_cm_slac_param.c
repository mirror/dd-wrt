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
 *   signed pev_cm_slac_param (struct session * session, struct channel * channel, struct message * message);
 *
 *   slac.h
 *
 *   PEV-HLE broadcasts cm_slac_param requests until at least one
 *   matching cm_slac_param confirm is received; matching confirm 
 *   has the same run identifier; EVSE-HLE returns information to 
 *   the PEV-HLE;
 *
 *   return 0 on success, -1 on error and 1 on timeout;
 *
 *   this interaction effectivey synchronizes the PEV-HLE and one
 *   or more EVSE-HLEs by initiating the SLAC protocol sequence;
 *
 *   MSOUND-TARGET will be FF:FF:FF:FF:FF:FF for SLAC;
 *   NUM_SOUNDS will be 8 for SLAC;
 *   TIME_OUT will be 10 for SLAC;
 *   RESP_TYPE will be 00 for SLAC (???);
 *   FORWARDING_STA will be 00:00:00:00:00:00 for SLAC when RESP_TYPE=0;
 *   APPLICATION_TYPE will be 0 for SLAC;
 *   SECUTITY_TYPE will be 0 for this application;
 *   RunID will be that defined by PEV-HLE;
 *
 *   send cm_slac_param request then wait up to channel.timeout 
 *   milliseconds for a cm_slac_param confirm having the same 
 *   RunID as the request; the application type and security 
 *   type should be the same as in the request; 
 *
 *   save the msound target, forwarding station, number of sounds, 
 *   sounding timeout and response type for use later in the session; 
 *
 *   the forwarding stations is a vague concept; the specification 
 *   authors say it should be FF:FF:FF:FF:FF:FF but here, the EVSE-HLE
 *   will set it to the PEV MAC;
 *   
 *--------------------------------------------------------------------*/

#ifndef PEV_CM_SLAC_PARAM_SOURCE
#define PEV_CM_SLAC_PARAM_SOURCE

#include <string.h>
#include <errno.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../iso15118/slac.h"

signed pev_cm_slac_param (struct session * session, struct channel * channel, struct message * message) 

{ 
	extern byte const broadcast [ETHER_ADDR_LEN]; 
	struct cm_slac_param_request * request = (struct cm_slac_param_request *) (message); 
	struct cm_slac_param_confirm * confirm = (struct cm_slac_param_confirm *) (message); 
	slac_debug (session, 0, __func__, "--> CM_SLAC_PARAM.REQ"); 
	memset (message, 0, sizeof (* message)); 
	EthernetHeader (& request->ethernet, broadcast, channel->host, channel->type); 
	HomePlugHeader1 (& request->homeplug, HOMEPLUG_MMV, (CM_SLAC_PARAM | MMTYPE_REQ)); 
	request->APPLICATION_TYPE = session->APPLICATION_TYPE; 
	request->SECURITY_TYPE = session->SECURITY_TYPE; 
	memcpy (request->RunID, session->RunID, sizeof (request->RunID)); 
	request->CipherSuite [0] = HTOLE16 ((uint16_t) (session->counter)); 
	if (sendmessage (channel, message, (ETHER_MIN_LEN - ETHER_CRC_LEN)) <= 0) 
	{ 
		return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
	} 
	while (readmessage (channel, message, HOMEPLUG_MMV, (CM_SLAC_PARAM | MMTYPE_CNF)) > 0) 
	{ 
		if (! memcmp (session->RunID, confirm->RunID, sizeof (session->RunID))) 
		{ 
			slac_debug (session, 0, __func__, "<-- CM_SLAC_PARAM.CNF"); 
			if (confirm->APPLICATION_TYPE != session->APPLICATION_TYPE) 
			{ 
				slac_debug (session, session->exit, __func__, "Unexpected APPLICATION_TYPE"); 
			} 
			if (confirm->SECURITY_TYPE != session->SECURITY_TYPE) 
			{ 
				slac_debug (session, session->exit, __func__, "Unexpected SECURITY_TYPE"); 
			} 
			if (_anyset (session->flags, SLAC_COMPARE)) 
			{ 
				if (LE16TOH (confirm->CipherSuite) != (uint16_t) (session->counter)) 
				{ 
					slac_debug (session, session->exit, __func__, "session->counter mismatch! PEV=(%d) EVSE=(%d)", LE16TOH (confirm->CipherSuite), session->counter); 
				} 
			} 

#if SLAC_DEBUG

			if (_anyset (session->flags, SLAC_VERBOSE)) 
			{ 
				char string [256]; 
				slac_debug (session, 0, __func__, "CM_SLAC_PARAM.CNF.MSOUND_TARGET %s", HEXSTRING (string, confirm->MSOUND_TARGET)); 
				slac_debug (session, 0, __func__, "CM_SLAC_PARAM.CNF.NUM_SOUNDS %d", confirm->NUM_SOUNDS); 
				slac_debug (session, 0, __func__, "CM_SLAC_PARAM.CNF.TIME_OUT %d", confirm->TIME_OUT); 
				slac_debug (session, 0, __func__, "CM_SLAC_PARAM.CNF.RESP_TYPE %d", confirm->RESP_TYPE); 
				slac_debug (session, 0, __func__, "CM_SLAC_PARAM.CNF.FORWARDING_STA %s", HEXSTRING (string, confirm->FORWARDING_STA)); 
				slac_debug (session, 0, __func__, "CM_SLAC_PARAM.CNF.APPLICATION_TYPE %d", confirm->APPLICATION_TYPE); 
				slac_debug (session, 0, __func__, "CM_SLAC_PARAM.CNF.SECURITY_TYPE %d", confirm->SECURITY_TYPE); 
				slac_debug (session, 0, __func__, "CM_SLAC_PARAM.CNF.RunID %s", HEXSTRING (string, confirm->RunID)); 
				slac_debug (session, 0, __func__, "CM_SLAC_PARAM.CNF.CipherSuite %d", confirm->CipherSuite); 
			} 

#endif

			memcpy (session->FORWARDING_STA, confirm->FORWARDING_STA, sizeof (session->FORWARDING_STA)); 
			memcpy (session->MSOUND_TARGET, confirm->MSOUND_TARGET, sizeof (session->MSOUND_TARGET)); 
			session->NUM_SOUNDS = confirm->NUM_SOUNDS; 
			session->TIME_OUT = confirm->TIME_OUT; 
			session->RESP_TYPE = confirm->RESP_TYPE; 
			return (0); 
		} 
	} 
	return (slac_debug (session, 0, __func__, "<-- CM_SLAC_PARAM.CNF ?")); 
} 

#endif



