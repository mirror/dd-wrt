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
 *   signed pev_cm_start_atten_char (struct session * session, struct channel * channel, struct message * message);
 *
 *   slac.h
 *   
 *   the PEV-HLE broadcasts a CM_START_ATTEN_CHAR.IND to initiate a 
 *   SLAC session; we broadcast three times per HPGP specification;
 *
 *--------------------------------------------------------------------*/

#ifndef PEV_CM_START_ATTEN_CHAR_SOURCE
#define PEV_CM_START_ATTEN_CHAR_SOURCE

#include <sys/time.h>
#include <memory.h>
#include <errno.h>

#include "../tools/types.h"
#include "../tools/error.h"
#include "../ether/channel.h"
#include "../slac/slac.h"

signed pev_cm_start_atten_char (struct session * session, struct channel * channel, struct message * message) 

{ 
	struct cm_start_atten_char_indicate * indicate = (struct cm_start_atten_char_indicate *) (message); 
	slac_debug (session, 0, __func__, "--> CM_START_ATTEN_CHAR.IND"); 
	memset (message, 0, sizeof (* message)); 
	EthernetHeader (& indicate->ethernet, session->MSOUND_TARGET, channel->host, channel->type); 
	HomePlugHeader1 (& indicate->homeplug, HOMEPLUG_MMV, (CM_START_ATTEN_CHAR | MMTYPE_IND)); 
	indicate->APPLICATION_TYPE = session->APPLICATION_TYPE; 
	indicate->SECURITY_TYPE = session->SECURITY_TYPE; 
	indicate->ACVarField.NUM_SOUNDS = session->NUM_SOUNDS; 
	indicate->ACVarField.TIME_OUT = session->TIME_OUT; 
	indicate->ACVarField.RESP_TYPE = session->RESP_TYPE; 
	memcpy (indicate->ACVarField.FORWARDING_STA, session->FORWARDING_STA, sizeof (indicate->ACVarField.FORWARDING_STA)); 
	memcpy (indicate->ACVarField.RunID, session->RunID, sizeof (indicate->ACVarField.RunID)); 
	if (sendmessage (channel, message, (ETHER_MIN_LEN - ETHER_CRC_LEN)) <= 0) 
	{ 
		return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
	} 

#if 0

/*	
 *	the GreenPHY spec says to send CM_START_ATTEN.IND three times to ensure
 *	that is is received;
 */

	if (sendmessage (channel, message, (ETHER_MIN_LEN - ETHER_CRC_LEN)) <= 0) 
	{ 
		return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
	} 
	if (sendmessage (channel, message, (ETHER_MIN_LEN - ETHER_CRC_LEN)) <= 0) 
	{ 
		return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
	} 

#endif

	return (0); 
} 

#endif



