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
 *   signed evse_cm_start_atten_char (struct session * session, struct channel * channel, struct message * message);
 *
 *   slac.h
 *
 *   EVSE_HLE stores NUM_SOUNDS, TIME_OUT and FORWARDING_STA for use
 *   later in the SLAC session;
 *
 *   accept only CM_ATTEN_CHAR.IND with RunID that match the earlier
 *   CM_SLAC_PARAM.REQ;
 *
 *--------------------------------------------------------------------*/

#ifndef EVSE_CM_START_ATTEN_CHAR_SOURCE
#define EVSE_CM_START_ATTEN_CHAR_SOURCE

#include <string.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../iso15118/slac.h"

signed evse_cm_start_atten_char (struct session * session, struct channel * channel, struct message * message) 

{ 
	struct cm_start_atten_char_indicate * indicate = (struct cm_start_atten_char_indicate *) (message); 
	if (readmessage (channel, message, HOMEPLUG_MMV, (CM_START_ATTEN_CHAR | MMTYPE_IND)) > 0) 
	{ 
		if (! memcmp (session->RunID, indicate->ACVarField.RunID, sizeof (session->RunID))) 
		{ 
			slac_debug (session, 0, __func__, "<-- CM_START_ATTEN_CHAR.IND"); 

#if SLAC_DEBUG

			if (_anyset (session->flags, SLAC_VERBOSE)) 
			{ 
				char string [256]; 
				slac_debug (session, 0, __func__, "CM_START_ATTEN_CHAR.IND.APPLICATION_TYPE %d", indicate->APPLICATION_TYPE); 
				slac_debug (session, 0, __func__, "CM_START_ATTEN_CHAR.IND.SECURITY_TYPE %d", indicate->SECURITY_TYPE); 
				slac_debug (session, 0, __func__, "CM_START_ATTEN_CHAR.IND.ACVarField.NUM_SOUNDS %d", indicate->ACVarField.NUM_SOUNDS); 
				slac_debug (session, 0, __func__, "CM_START_ATTEN_CHAR.IND.ACVarField.TIME_OUT %d", indicate->ACVarField.TIME_OUT); 
				slac_debug (session, 0, __func__, "CM_START_ATTEN_CHAR.IND.ACVarField.RESP_TYPE %d", indicate->ACVarField.RESP_TYPE); 
				slac_debug (session, 0, __func__, "CM_START_ATTEN_CHAR.IND.ACVarField.FORWARDING_STA %s", HEXSTRING (string, indicate->ACVarField.FORWARDING_STA)); 
				slac_debug (session, 0, __func__, "CM_START_ATTEN_CHAR.IND.ACVarField.RunID %s", HEXSTRING (string, indicate->ACVarField.RunID)); 
			} 

#endif

			if (indicate->APPLICATION_TYPE != session->APPLICATION_TYPE) 
			{ 
				slac_debug (session, session->exit, __func__, "%s: APPLICATION_TYPE", __func__); 
			} 
			if (indicate->SECURITY_TYPE != session->SECURITY_TYPE) 
			{ 
				slac_debug (session, session->exit, __func__, "%s: SECURITY_TYPE", __func__); 
			} 
			session->NUM_SOUNDS = indicate->ACVarField.NUM_SOUNDS; 
			session->TIME_OUT = indicate->ACVarField.TIME_OUT; 
			if (indicate->ACVarField.RESP_TYPE != session->RESP_TYPE) 
			{ 
				slac_debug (session, session->exit, __func__, "%s: RESP_TYPE", __func__); 
			} 
			memcpy (session->FORWARDING_STA, indicate->ACVarField.FORWARDING_STA, sizeof (session->FORWARDING_STA)); 
			return (0); 
		} 
	} 
	return (slac_debug (session, session->exit, __func__, "CM_START_ATTEN_CHAR.IND ?")); 
} 

#endif



