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
 *   signed pev_cm_mnbc_sound (struct session * session, struct channel * channel, struct message * message);
 *
 *   slac.h
 *
 *   As HLE-PEV send cm_mnbc_sound indications unicast to the
 *   MSOUND_TARGET address recorded in the session variable;
 *
 *   a brief delay of a few milliseconds is needed between msounds 
 *   so that EVSE-PLC has time to forward CM_MNBC_SOUND.IND and
 *   CM_ATTEN_PROFILE.IND to EVSE-HLE; session.timer controls this
 *   delay;
 *
 *--------------------------------------------------------------------*/

#ifndef PEV_CM_MNBC_SOUND_SOURCE
#define PEV_CM_MNBC_SOUND_SOURCE

#include <string.h>
#include <errno.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/timer.h"
#include "../slac/slac.h"

signed pev_cm_mnbc_sound (struct session * session, struct channel * channel, struct message * message) 

{ 
	struct cm_mnbc_sound_indicate * indicate = (struct cm_mnbc_sound_indicate *) (message); 
	signed sound = session->NUM_SOUNDS; 
	while (sound--) 
	{ 
		slac_debug (session, 0, __func__, "--> CM_MNBC_SOUND.IND"); 
		memset (message, 0, sizeof (* message)); 
		EthernetHeader (& indicate->ethernet, session->MSOUND_TARGET, channel->host, channel->type); 
		HomePlugHeader1 (& indicate->homeplug, HOMEPLUG_MMV, (CM_MNBC_SOUND | MMTYPE_IND)); 
		indicate->APPLICATION_TYPE = session->APPLICATION_TYPE; 
		indicate->SECURITY_TYPE = session->SECURITY_TYPE; 
		memcpy (indicate->MSVarField.SenderID, session->PEV_ID, sizeof (indicate->MSVarField.SenderID)); 
		indicate->MSVarField.CNT = sound; 
		memcpy (indicate->MSVarField.RunID, session->RunID, sizeof (indicate->MSVarField.RunID)); 
		memset (indicate->MSVarField.RND, 0, sizeof (indicate->MSVarField.RND)); 
		if (sendmessage (channel, message, sizeof (* indicate)) <= 0) 
		{ 
			return (slac_debug (session, 1, __func__, CHANNEL_CANTSEND)); 
		} 
		SLEEP (session->pause); 
	} 
	return (0); 
} 

#endif



