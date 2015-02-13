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
 *   signed PrintRule (uint32_t CR_PID, uint32_t CR_OPERAND, uint8_t CR_VALUE []);
 *
 *   rules.h
 *
 *   print one classifier rule on stdout;
 *
 *   some classifier rule structures have an 8-bit PID and OPERAND
 *   while others have a 32-bit PID and OPERAND; this function is
 *   the common demominator;
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef PRINTRULE_SOURCE
#define PRINTRULE_SOURCE

#include <string.h>

#include "../plc/rules.h"
#include "../tools/error.h"

signed PrintRule (uint32_t CR_PID, uint32_t CR_OPERAND, uint8_t CR_VALUE [])

{
	char buffer [CLASSIFIER_STRING];
	uint32_t val32;
	uint16_t val16;
	uint8_t val8;
	const char * p1;
	const char * p2;
	p1 = reword (CR_PID, fields, SIZEOF (fields));
	if (p1 == NULL)
	{
		error (1, 0, "invalid classifier PID");
	}
	p2 = reword (CR_OPERAND, operators, SIZEOF (operators));
	if (p2 == NULL)
	{
		error (1, 0, "invalid classifier operand");
	}
	printf ("%s %s", p1, p2);
	switch (CR_PID)
	{
	case FIELD_ETH_SA:
	case FIELD_ETH_DA:
		printf (" %s", hexstring (buffer, sizeof (buffer), CR_VALUE, ETHER_ADDR_LEN));
		break;
	case FIELD_IPV4_SA:
	case FIELD_IPV4_DA:
		putchar (' ');
		memout (CR_VALUE, IPv4_LEN, "%d", 1, '.', 0, stdout);
		break;
	case FIELD_IPV6_SA:
	case FIELD_IPV6_DA:
		putchar (' ');
		memout (CR_VALUE, IPv6_LEN, "%02x", 2, ':', 0, stdout);
		break;
	case FIELD_VLAN_UP:
	case FIELD_IPV6_TC:
	case FIELD_IPV4_TOS:
	case FIELD_IPV4_PROT:
		memcpy (& val8, CR_VALUE, sizeof (val8));
		printf (" 0x%02X", val8);
		break;
	case FIELD_VLAN_ID:
	case FIELD_TCP_SP:
	case FIELD_TCP_DP:
	case FIELD_UDP_SP:
	case FIELD_UDP_DP:
	case FIELD_IP_SP:
	case FIELD_IP_DP:
		memcpy (& val16, CR_VALUE, sizeof (val16));
		val16 = ntohs (val16);
		printf (" %d", val16);
		break;
	case FIELD_ETH_TYPE:
		memcpy (& val16, CR_VALUE, sizeof (val16));
		val16 = ntohs (val16);
		printf (" 0x%04X", val16);
		break;
	case FIELD_IPV6_FL:
		memcpy (& val32, & CR_VALUE [0], sizeof (val32));
		val32 = ntohl (val32);
		printf (" 0x%08X", val32);
		break;
	case FIELD_HPAV_MME:
		memcpy (& val8, & CR_VALUE [0], sizeof (val8));
		memcpy (& val16, & CR_VALUE [1], sizeof (val16));
		printf (" %02x:%04x", val8, val16);
		break;
	case FIELD_TCP_ACK:
		{
			code_t val;
			const char * p;
			memcpy (& val, CR_VALUE, sizeof (val));
			p = reword (val, states, SIZEOF (states));
			if (p == NULL)
			{
				error (1, 0, "invalid TCP ACK flag");
			}
			printf (" %s", p);
		}
		break;
	case FIELD_VLAN_TAG:
		{
			code_t val;
			const char * p;
			memcpy (& val, CR_VALUE, sizeof (val));
			p = reword (val, states, SIZEOF (states));
			if (p == NULL)
			{
				error (1, 0, "invalid VLAN tag flag");
			}
			printf (" %s", p);
		}
		break;
	default:
		printf (" *****UNSUPPORTED CODE*****");
		break;
	}
	return (0);
}

#endif

