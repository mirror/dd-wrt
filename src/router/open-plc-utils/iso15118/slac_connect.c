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
 *   void slac_connect (struct session * session);
 *   
 *   slac.h
 *
 *   compute the arithmetic mean of session attenuation values and
 *   compare to the session limit; average attenuation greater than
 *   the session limit is considered a bad connection;
 *
 *--------------------------------------------------------------------*/

#ifndef SLAC_AVERAGE_SOURCE
#define SLAC_AVERAGE_SOURCE

#include "../tools/error.h"
#include "../tools/flags.h"
#include "../tools/memory.h"
#include "../iso15118/slac.h"

signed slac_connect (struct session * session) 

{ 
	unsigned group = 0; 
	unsigned total = 0; 
	if (session->NumGroups > SIZEOF (session->AAG)) 
	{ 
		return (slac_debug (session, session->exit, __func__, "Too much data to analyse!")); 
	} 
	if (session->NumGroups > 0) 
	{ 
		char string [512]; 
		while (group < session->NumGroups) 
		{ 
			total += session->AAG [group]; 
			group++; 
		} 
		total /= group; 
		if (total > session->limit) 
		{ 
			char string [512]; 
			slac_debug (session, 0, __func__, "Average attenuation (%u) more than limit (%u) frow %d groups", total, session->limit, group); 
			slac_debug (session, 0, __func__, "%s", HEXSTRING (string, session->AAG)); 
			return (- 1); 
		} 
		if (total > 0) 
		{ 
			slac_debug (session, 0, __func__, "Average attenuation (%u) less than limit (%u) from %d groups", total, session->limit, group); 
			slac_debug (session, 0, __func__, "%s", HEXSTRING (string, session->AAG)); 
			return (0); 
		} 
	} 
	return (slac_debug (session, session->exit, __func__, "Nothing to analyse")); 
} 

#endif



