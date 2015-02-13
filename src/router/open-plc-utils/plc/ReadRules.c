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
 *   signed ReadRules (struct plc * plc);
 *
 *   plc.h
 *
 *   Read and Display the classifier rules read from a PLC device.
 *
 *   Contributor(s):
 *      Nathan Houghton <nhoughto@qca.qualcomm.com>
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef READRULES_SOURCE
#define READRULES_SOURCE

#include <limits.h>
#include <stdio.h>

#include "../plc/plc.h"
#include "../plc/rules.h"
#include "../tools/error.h"

signed ReadRules (struct plc * plc)

{
	struct channel * channel = (struct channel *) (plc->channel);
	struct message * message = (struct message *) (plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_classification_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MCONTROL;
		uint8_t RSVD;
		uint8_t OFFSET;
		uint8_t COUNT;
	}
	* request = (struct vs_classification_request *) (message);
	struct __packed vs_classification_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t TOTAL_CLASSIFIERS;
		uint8_t OFFSET;
		uint8_t COUNT;
		struct __packed MMEReadRule
		{
			uint8_t MACTION;
			uint8_t MOPERAND;
			uint8_t NUM_CLASSIFIERS;
			struct MMEClassifier CLASSIFIER [3];
			struct cspec cspec;
		}
		RULESET [60];
	}
	* confirm = (struct vs_classification_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	unsigned index = 0;
	unsigned total = UINT_MAX;
	Request (plc, "Read Classifier Rules");
	while (index < total)
	{
		struct MMEReadRule * rule;
		memset (message, 0, sizeof (* message));
		EthernetHeader (& request->ethernet, channel->peer, channel->host, channel->type);
		QualcommHeader (& request->qualcomm, 0, (VS_CLASSIFICATION | MMTYPE_REQ));
		plc->packetsize = ETHER_MIN_LEN - ETHER_CRC_LEN;
		request->MCONTROL = CONTROL_READ;
		request->OFFSET = index;
		request->COUNT = SIZEOF (confirm->RULESET);
		if (SendMME (plc) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
			return (-1);
		}
		if (ReadMME (plc, 0, (VS_CLASSIFICATION | MMTYPE_CNF)) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
			return (-1);
		}
		if (confirm->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			return (-1);
		}
		total = confirm->TOTAL_CLASSIFIERS;
		index += confirm->COUNT;
		rule = confirm->RULESET;
		while (confirm->COUNT--)
		{
			int count;
			int rule_len;
			const char * p1;
			const char * p2;
			struct cspec * cspec;
			if (rule->NUM_CLASSIFIERS > RULE_MAX_CLASSIFIERS)
			{
				error (1, 0, "too many classifiers in rule (%d, expecting <= %d)", rule->NUM_CLASSIFIERS, RULE_MAX_CLASSIFIERS);
			}
			rule_len = sizeof (* rule) - (RULE_MAX_CLASSIFIERS - rule->NUM_CLASSIFIERS) * sizeof (struct MMEClassifier) - sizeof (struct cspec);
			if (rule->MACTION == ACTION_AUTOCONNECT || rule->MACTION == ACTION_TAGTX || rule->MACTION == ACTION_TAGRX)
			{
				cspec = (struct cspec *) ((uint8_t *) rule +  rule_len);
				rule_len += sizeof (struct cspec);
			}
			if (rule->MACTION == ACTION_TAGTX)
			{
				printf ("-T 0x%08X -V %d ", ntohl (cspec->VLAN_TAG), cspec->CSPEC_VERSION);
			}
			p1 = reword (rule->MACTION, actions, CLASSIFIER_ACTIONS);
			if (p1 == NULL)
			{
				error (1, 0, "invalid classifier action");
			}
			p2 = reword (rule->MOPERAND, operands, CLASSIFIER_OPERANDS);
			if (p2 == NULL)
			{
				error (1, 0, "invalid classifier operand");
			}
			printf ("%s", p1);
			printf (" %s ", p2);

/* need to dump out the actual conditions here */

			for (count = 0; count < rule->NUM_CLASSIFIERS; ++ count)
			{
				struct MMEClassifier * classifier = & rule->CLASSIFIER [count];
				PrintRule (classifier->CR_PID, classifier->CR_OPERAND, classifier->CR_VALUE);
				putchar (' ');
			}
			printf ("add temp\n");
			rule = (struct MMEReadRule *) ((uint8_t *) (rule) +  rule_len);
		}
	}
	return (0);
}

#endif



