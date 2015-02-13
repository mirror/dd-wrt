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
 *   signed ParseRule (int * argcp, char const * argvp [], struct rule * rule, struct cspec * cspec);
 *
 *   rules.h
 *
 *   This module takes an argument vector and an argument count
 *   and fills in a classification rule structure that is suitable for
 *   sending in a VS_CLASSIFICATION MME;
 *
 *   This module is currently used by plcrule and pibruin;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#include <memory.h>
#include <errno.h>

#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../ether/ether.h"
#include "../plc/rules.h"

signed ParseRule (int * argcp, char const ** argvp [], struct MMERule * rule, struct cspec * cspec)

{
	int argc = * argcp;
	char const ** argv = * argvp;
	union 
	{
		uint32_t wide;
		uint16_t word;
		uint8_t byte [4];
	}
	temp;
	signed code;
	struct MMEClassifier * classifier = (struct MMEClassifier *) (& rule->CLASSIFIER);
	if ((code = lookup (* argv++, actions, SIZEOF (actions))) == -1)
	{
		assist (* -- argv, CLASSIFIER_ACTION_NAME, actions, SIZEOF (actions));
	}
	rule->MACTION = (uint8_t) (code);
	argc--;
	if ((code = lookup (* argv++, operands, SIZEOF (operands))) == -1)
	{
		assist (* -- argv, CLASSIFIER_OPERAND_NAME, operands, SIZEOF (operands));
	}
	rule->MOPERAND = (uint8_t) (code);
	argc--;
	while ((* argv) && (lookup (* argv, controls, SIZEOF (controls)) == -1))
	{
		if ((code = lookup (* argv++, fields, SIZEOF (fields))) == -1)
		{
			assist (* -- argv, CLASSIFIER_FIELD_NAME, fields, SIZEOF (fields));
		}
		classifier->CR_PID = (uint8_t) (code);
		argc--;
		if ((code = lookup (* argv++, operators, SIZEOF (operators))) == -1)
		{
			assist (* -- argv, CLASSIFIER_OPERATOR_NAME, operators, SIZEOF (operators));
		}
		classifier->CR_OPERAND = (uint8_t) (code);
		argc--;
		if (! argc || ! * argv)
		{
			error (1, ENOTSUP, "I have %s '%s' but no value", CLASSIFIER_OPERATOR_NAME, * -- argv);
		}
		switch (classifier->CR_PID)
		{
		case FIELD_ETH_SA:
		case FIELD_ETH_DA:
			bytespec (* argv++, classifier->CR_VALUE, ETHER_ADDR_LEN);
			break;
		case FIELD_IPV4_SA:
		case FIELD_IPV4_DA:
			ipv4spec (* argv++, classifier->CR_VALUE);
			break;
		case FIELD_IPV6_SA:
		case FIELD_IPV6_DA:
			ipv6spec (* argv++, classifier->CR_VALUE);
			break;
		case FIELD_VLAN_UP:
		case FIELD_IPV6_TC:
		case FIELD_IPV4_TOS:
		case FIELD_IPV4_PROT:
			classifier->CR_VALUE [0] = (uint8_t) (basespec (* argv++, 0, sizeof (classifier->CR_VALUE [0])));
			break;
		case FIELD_VLAN_ID:
		case FIELD_TCP_SP:
		case FIELD_TCP_DP:
		case FIELD_UDP_SP:
		case FIELD_UDP_DP:
		case FIELD_IP_SP:
		case FIELD_IP_DP:
			temp.word = (uint16_t) (basespec (* argv++, 0, sizeof (temp.word)));
			temp.word = htons (temp.word);
			memcpy (classifier->CR_VALUE, & temp, sizeof (temp.word));
			break;
		case FIELD_ETH_TYPE:
			temp.word = (uint16_t) (basespec (* argv++, 0, sizeof (temp.word)));
			temp.word = htons (temp.word);
			memcpy (classifier->CR_VALUE, & temp, sizeof (temp.word));
			break;
		case FIELD_IPV6_FL:
			temp.wide = (uint32_t) (basespec (* argv++, 0, sizeof (temp.wide))) & 0x000FFFFF;
			temp.wide = htonl (temp.wide);
			memcpy (classifier->CR_VALUE, & temp.byte [1], 3);
			break;
		case FIELD_HPAV_MME:
			bytespec (* argv++, classifier->CR_VALUE, sizeof (uint16_t) +  sizeof (uint8_t));
			temp.byte [0] = classifier->CR_VALUE [1];
			classifier->CR_VALUE [1] = classifier->CR_VALUE [2];
			classifier->CR_VALUE [2] = temp.byte [0];
			break;
		case FIELD_TCP_ACK:
			if ((code = lookup (* argv++, states, SIZEOF (states))) == -1)
			{
				assist (* -- argv, CLASSIFIER_STATE_NAME, states, SIZEOF (states));
			}
			memset (classifier->CR_VALUE, 0, sizeof (classifier->CR_VALUE));
			break;
		case FIELD_VLAN_TAG:
			if ((code = lookup (* argv++, states, SIZEOF (states))) == -1)
			{
				assist (* -- argv, CLASSIFIER_STATE_NAME, states, SIZEOF (states));
			}
			memset (classifier->CR_VALUE, 0, sizeof (classifier->CR_VALUE));
			classifier->CR_OPERAND ^= code;
			break;
		default: 
			error (1, ENOTSUP, "%s", argv [- 2]);
			break;
		}
		rule->NUM_CLASSIFIERS++;
		classifier++;
		argc--;
	}
	memcpy (classifier, cspec, sizeof (* cspec));
	if ((code = lookup (* argv++, controls, SIZEOF (controls))) == -1)
	{
		assist (* -- argv, CLASSIFIER_CONTROL_NAME, controls, SIZEOF (controls));
	}
	rule->MCONTROL = (uint8_t) (code);
	argc--;
	if ((code = lookup (* argv++, volatilities, SIZEOF (volatilities))) == -1)
	{
		assist (* -- argv, CLASSIFIER_VOLATILITY_NAME, volatilities, SIZEOF (volatilities));
	}
	rule->MVOLATILITY = (uint8_t) (code);
	argc--;
	* argcp = argc;
	* argvp = argv;
	return (0);
}

