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
 *   evse.c - QCA Electric Vehicle Supply Equipment Emulator;
 *
 *   This program, in the current state, is not a finished product;
 *   It has been released so that interested parties can begin to 
 *   see how the SLAC protocol might be implemented;
 *
 *   Some key design features are:
 *
 *   1) the use of a channel variable to abstract ISO Layer 2 I/O;
 *      the variable is used by functions openchannel, readmessage,
 *      sendmessage and closechannel;
 *
 *   2) the use of a message variable to represent an IEEE 802.3 
 *      Ethernet frame; the variable allows one frame to be used
 *      and re-used throughout the program but supports multiple
 *      frame buffers if needed;
 *
 *   3) the use of a session variable to support multiple PEV-EVSE
 *      interactions without using threads or subrocesses; this has
 *      not demonstrated in this version of the program; some more
 *      work is needed;
 *
 *   4) the absence of threads or subprocesses so that the  program 
 *      can be ported to hosts without a multi-tasking operating 
 *      system;
 *
 *   5) lots of debugging messages; these can be suppressed or 
 *      deleted if not wanted;
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/time.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../ether/channel.h"
#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/config.h"
#include "../tools/types.h"
#include "../tools/timer.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../ether/channel.h"
#include "../slac/slac.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/hexdecode.c"
#include "../tools/hexencode.c"
#include "../tools/hexdump.c"
#include "../tools/hexout.c"
#include "../tools/hexstring.c"
#include "../tools/decdecode.c"
#include "../tools/decstring.c"
#include "../tools/todigit.c"
#include "../tools/strfbits.c"
#include "../tools/error.c"
#include "../tools/config.c"
#endif

#ifndef MAKEFILE
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#include "../ether/channel.c"
#endif

#ifndef MAKEFILE
#include "../plc/Devices.c"
#endif

#ifndef MAKEFILE
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/HomePlugHeader1.c"
#include "../mme/UnwantedMessage.c"
#include "../mme/readmessage.c"
#include "../mme/sendmessage.c"
#endif

#ifndef MAKEFILE
#include "../slac/slac_session.c"
#include "../slac/slac_debug.c"
#include "../slac/evse_cm_slac_param.c"
#include "../slac/evse_cm_start_atten_char.c"
#include "../slac/evse_cm_atten_char.c"
#include "../slac/evse_cm_mnbc_sound.c"
#include "../slac/evse_cm_slac_match.c"
#include "../slac/evse_cm_set_key.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define PLCDEVICE "PLC"
#define PROFILE "evse.ini"
#define SECTION "default" 
#define STATION ""

#define EVSE_STATE_UNAVAILABLE 0
#define EVSE_STATE_UNOCCUPIED 1
#define EVSE_STATE_UNMATCHED 2
#define EVSE_STATE_MATCHED 3

#define EVSE_SID "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"	// Station Identifier
#define EVSE_NMK "B59319D7E8157BA001B018669CCEE30D" 	// HomePlugAV0123
#define EVSE_NID "026BCBA5354E08"		    	// HomePlugAV0123


/*====================================================================*
 *
 *   void configure ()
 *
 *   print default EVSE-HLE configuration file on stdout so that the
 *   profile, section and element names match; 
 *
 *--------------------------------------------------------------------*/

static void configure () 

{ 
	printf ("# file: %s\n", PROFILE); 
	printf ("# ====================================================================\n"); 
	printf ("# EVSE-HLE initiaization;\n"); 
	printf ("# --------------------------------------------------------------------\n"); 
	printf ("[%s]\n", SECTION); 
	printf ("station identifier = %s\n", EVSE_SID); 
	printf ("network membership key = %s\n", EVSE_NMK); 
	printf ("network identifier = %s\n", EVSE_NID); 
	printf ("number of sounds = %d\n", SLAC_MSOUNDS); 
	printf ("time to sound = %d\n", SLAC_TIMETOSOUND); 
	printf ("response type = %d\n", SLAC_RESPONSE_TYPE); 
	printf ("settle time = %d\n", SLAC_SETTLETIME); 
	printf ("charge time = %d\n", SLAC_CHARGETIME); 
	return; 
} 

/*====================================================================*
 *
 *   char void initialize (struct session * session, char const * profile, char const * section) 
 *
 *   read EVSE-HLE configuration profile; initialize session variable;
 *   
 *--------------------------------------------------------------------*/

static void initialize (struct session * session, char const * profile, char const * section) 

{ 
	session->next = session->prev = session; 
	hexencode (session->EVSE_ID, sizeof (session->EVSE_ID), configstring (profile, section, "StationIdentifier", EVSE_SID)); 
	hexencode (session->NMK, sizeof (session->NMK), configstring (profile, section, "NetworkMembershipKey", EVSE_NMK)); 
	hexencode (session->NID, sizeof (session->NID), configstring (profile, section, "NetworkIdentifier", EVSE_NID)); 
	session->NUM_SOUNDS = confignumber_range (profile, section, "NumberOfSounds", SLAC_MSOUNDS, 0, UCHAR_MAX); 
	session->TIME_OUT = confignumber_range (profile, section, "TimeToSound", SLAC_TIMETOSOUND, 0, UCHAR_MAX); 
	session->RESP_TYPE = confignumber_range (profile, section, "ResponseType", SLAC_RESPONSE_TYPE, 0, UCHAR_MAX); 
	session->chargetime = confignumber_range (profile, section, "ChargeTime", SLAC_CHARGETIME, 0, UINT_MAX); 
	session->settletime = confignumber_range (profile, section, "SettleTime", SLAC_SETTLETIME, 0, UINT_MAX); 
	memcpy (session->original_nmk, session->NMK, sizeof (session->original_nmk)); 
	memcpy (session->original_nid, session->NID, sizeof (session->original_nid)); 
	session->state = EVSE_STATE_UNOCCUPIED; 
	slac_session (session); 
	return; 
} 

/*====================================================================*
 *
 *   signed identifier (struct session * session, struct channel * channel);
 *
 *   copy channel host address to session EVSE MAC address; set session
 *   EVSE identifier to zeros;
 *
 *--------------------------------------------------------------------*/

static signed identifier (struct session * session, struct channel * channel) 

{ 
	memcpy (session->EVSE_MAC, channel->host, sizeof (session->EVSE_MAC)); 
	return (0); 
} 

/*====================================================================*
 *
 *   void UnoccupiedState (struct session * session, struct channel * channel, struct message * message);
 *
 *--------------------------------------------------------------------*/

static void UnoccupiedState (struct session * session, struct channel * channel, struct message * message) 

{ 
	slac_session (session); 
	slac_debug (session, 0, __func__, "Listening ..."); 
	while (evse_cm_slac_param (session, channel, message)); 
	session->state = EVSE_STATE_UNMATCHED; 
	return; 
} 

/*====================================================================*
 *
 *   void MatchingState (struct session * session, struct channel * channel, struct message * message);
 *
 *   the cm_start_atten_char message establishes msound count and 
 *   timeout;
 *
 *--------------------------------------------------------------------*/

static void UnmatchedState (struct session * session, struct channel * channel, struct message * message) 

{ 
	slac_session (session); 
	slac_debug (session, 0, __func__, "Sounding ..."); 
	if (evse_cm_start_atten_char (session, channel, message)) 
	{ 
		session->state = EVSE_STATE_UNOCCUPIED; 
		return; 
	} 
	if (evse_cm_mnbc_sound (session, channel, message)) 
	{ 
		session->state = EVSE_STATE_UNOCCUPIED; 
		return; 
	} 
	if (evse_cm_atten_char (session, channel, message)) 
	{ 
		session->state = EVSE_STATE_UNOCCUPIED; 
		return; 
	} 
	slac_debug (session, 0, __func__, "Matching ..."); 
	if (evse_cm_slac_match (session, channel, message)) 
	{ 
		session->state = EVSE_STATE_UNOCCUPIED; 
		return; 
	} 
	session->state = EVSE_STATE_MATCHED; 
	return; 
} 

/*====================================================================*
 *
 *   void MatchedState (struct session * session, struct channel * channel, struct message * message);
 *
 *--------------------------------------------------------------------*/

static void MatchedState (struct session * session, struct channel * channel, struct message * message) 

{ 
	slac_debug (session, 0, __func__, "Connecting ..."); 

#if SLAC_AVLN_EVSE

	if (evse_cm_set_key (session, channel, message)) 
	{ 
		session->state = EVSE_STATE_UNOCCUPIED; 
		return; 
	} 
	sleep (session->settletime); 

#endif
#if SLAC_AVLN_PEV

	slac_debug (session, 0, __func__, "waiting for pev to settle ..."); 
	sleep (session->settletime); 

#endif

	slac_debug (session, 0, __func__, "Charging (%d) ...\n\n", session->counter++); 
	sleep (session->chargetime); 
	slac_debug (session, 0, __func__, "Disconnecting ..."); 

#if SLAC_AVLN_EVSE

	memcpy (session->NMK, session->original_nmk, sizeof (session->NMK)); 
	memcpy (session->NID, session->original_nid, sizeof (session->NID)); 
	if (evse_cm_set_key (session, channel, message)) 
	{ 
		session->state = EVSE_STATE_UNOCCUPIED; 
		return; 
	} 
	sleep (session->settletime); 

#endif
#if SLAC_AVLN_PEV

	slac_debug (session, 0, __func__, "waiting for pev to settle ..."); 
	sleep (session->settletime); 

#endif

	session->state = EVSE_STATE_UNOCCUPIED; 
	return; 
} 

/*====================================================================*
 *   
 *   int main (int argc, char const * argv[]);
 *   
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv []) 

{ 
	extern struct channel channel; 
	static char const * optv [] = 
	{ 
		"cCdi:p:qs:t:vx", 
		"", 
		"Qualcomm Atheros Electric Vehicle Supply Equipment Emulator", 
		"c\tprint template configuration file on stdout", 
		"C\tstop on count mismatch", 
		"d\tdisplay debug information", 

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]", 

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]", 

#endif

		"p s\tconfiguration profile is (s) [" LITERAL (PROFILE) "]", 
		"s s\tconfiguration section is (s) [" LITERAL (SECTION) "]", 
		"q\tquiet mode", 
		"t n\tread timeout is (n) milliseconds [" LITERAL (SLAC_TIMEOUT) "]", 
		"v\tverbose mode", 
		"x\texit on error", 
		(char const *) (0)
	}; 
	struct session session; 
	struct message message; 
	char const * profile = PROFILE; 
	char const * section = SECTION; 
	signed c; 
	memset (& session, 0, sizeof (session)); 
	memset (& message, 0, sizeof (message)); 
	channel.timeout = SLAC_TIMEOUT; 
	if (getenv (PLCDEVICE)) 
	{ 

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (PLCDEVICE)); 

#else

		channel.ifname = strdup (getenv (PLCDEVICE)); 

#endif

	} 
	optind = 1; 
	while (~ (c = getoptv (argc, argv, optv))) 
	{ 
		switch (c) 
		{ 
		case 'c': 
			configure (); 
			return (0); 
		case 'C': 
			_setbits (session.flags, SLAC_COMPARE); 
			break; 
		case 'd': 
			_setbits (session.flags, (SLAC_VERBOSE | SLAC_SESSION)); 
			break; 
		case 'i': 

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg); 

#else

			channel.ifname = optarg; 

#endif

			break; 
		case 'p': 
			profile = optarg; 
			break; 
		case 's': 
			section = optarg; 
			break; 
		case 'q': 
			_setbits (channel.flags, CHANNEL_SILENCE);
			_setbits (session.flags, SLAC_SILENCE); 
			break; 
		case 't': 
			channel.timeout = (signed) (uintspec (optarg, 0, UINT_MAX)); 
			break; 
		case 'v': 
			_setbits (channel.flags, CHANNEL_VERBOSE); 
			break; 
		case 'x': 
			session.exit = session.exit? 0: 1; 
			break; 
		default: 
			break; 
		} 
	} 
	argc -= optind; 
	argv += optind; 
	if (argc) 
	{ 
		slac_debug (& session, 1, __func__, ERROR_TOOMANY); 
	} 
	openchannel (& channel); 
	initialize (& session, profile, section); 
	identifier (& session, & channel); 
	if (evse_cm_set_key (& session, & channel, & message)) 
	{ 
		slac_debug (& session, 1, __func__, "Can't set key."); 
	} 
	sleep (session.settletime); 
	while (session.state) 
	{ 
		if (session.state == EVSE_STATE_UNOCCUPIED) 
		{ 
			UnoccupiedState (& session, & channel, & message); 
			continue; 
		} 
		if (session.state == EVSE_STATE_UNMATCHED) 
		{ 
			UnmatchedState (& session, & channel, & message); 
			continue; 
		} 
		if (session.state == EVSE_STATE_MATCHED) 
		{ 
			MatchedState (& session, & channel, & message); 
			continue; 
		} 
		slac_debug (& session, 1, __func__, "Illegal state!"); 
	} 
	closechannel (& channel); 
	exit (0); 
} 

