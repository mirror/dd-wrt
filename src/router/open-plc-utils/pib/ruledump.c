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
 *   void ruledump (struct rule * rule, FILE * fp);
 *
 *
 *--------------------------------------------------------------------*/

#ifndef RULEDUMP_SOURCE
#define RULEDUMP_SOURCE

#include <stdio.h>

#include "../tools/memory.h"
#include "../plc/rules.h"

/*====================================================================*
 *
 *   cspec_dump (struct cspec * cspec);
 *
 *
 *--------------------------------------------------------------------*/

void cspec_dump (struct cspec * cspec)

{
	printf ("cspec->CSPEC_VERSION=%d\n", LE16TOH (cspec->CSPEC_VERSION));
	printf ("cspec->VLAN_TAG=%d\n", LE16TOH (cspec->VLAN_TAG));
	return;
}


/*====================================================================*
 *
 *   MMEClassifierDump (struct MMEClassifier * classifier);
 *
 *
 *--------------------------------------------------------------------*/

void MMEClassifierDump (struct MMEClassifier * classifier)

{
	char string [48];
	printf ("classifier->CR_PID=0x%02X\n", (unsigned)(classifier->CR_PID));
	printf ("classifier->CR_OPERAND=0x%02X\n", classifier->CR_OPERAND);
	hexdecode (classifier->CR_VALUE, sizeof (classifier->CR_VALUE), string, sizeof (string));
	printf ("classifier->CR_VALUE=%s\n", string);
	return;
}


/*====================================================================*
 *
 *   PIBClassifierDump (struct PIBClassifier * classifier);
 *
 *
 *--------------------------------------------------------------------*/

void PIBClassifierDump (struct PIBClassifier * classifier)

{
	char string [48];
	printf ("classifier->CR_PID=0x%08X\n", LE32TOH (classifier->CR_PID));
	printf ("classifier->CR_OPERAND=0x%08X\n", LE32TOH (classifier->CR_OPERAND));
	hexdecode (classifier->CR_VALUE, sizeof (classifier->CR_VALUE), string, sizeof (string));
	printf ("classifier->CR_VALUE=%s\n", string);
	return;
}


/*====================================================================*
 *
 *   MMERuleDump (struct MMERule * rule);
 *
 *
 *--------------------------------------------------------------------*/

void MMERuleDump (struct MMERule * rule)

{
	unsigned count;
	printf ("rule->MCONTROL=%d\n", rule->MCONTROL);
	printf ("rule->MVOLATILITY=%d\n", rule->MVOLATILITY);
	printf ("rule->MACTION=%d\n", rule->MACTION);
	printf ("rule->MOPERAND=%d\n", rule->MOPERAND);
	printf ("rule->NUM_CLASSIFIERS=%d\n", rule->NUM_CLASSIFIERS);
	for (count = 0; count < rule->NUM_CLASSIFIERS; count++)
	{
		MMEClassifierDump (&rule->CLASSIFIER [count]);
	}
	cspec_dump (&rule->cspec);
	printf ("\n");
	return;
}


/*====================================================================*
 *
 *   PIBRuleDump (struct PIBRule * rule);
 *
 *
 *--------------------------------------------------------------------*/

void PIBRuleDump (struct PIBRule * rule)

{
	unsigned count;
	printf ("rule->MCONTROL=%d\n", rule->MCONTROL);
	printf ("rule->MVOLATILITY=%d\n", rule->MVOLATILITY);
	printf ("rule->MACTION=%d\n", rule->MACTION);
	printf ("rule->MOPERAND=%d\n", rule->MOPERAND);
	printf ("rule->NUM_CLASSIFIERS=%d\n", LE32TOH (rule->NUM_CLASSIFIERS));
	for (count = 0; count < LE32TOH (rule->NUM_CLASSIFIERS); count++)
	{
		PIBClassifierDump (&rule->CLASSIFIER [count]);
	}
	cspec_dump (&rule->cspec);
	printf ("\n");
	return;
}


/*====================================================================*
 *
 *   classifier_priority_map_dump (struct classifier_priority_map * classifier_priority_map);
 *
 *
 *--------------------------------------------------------------------*/

void classifier_priority_map_dump (struct classifier_priority_map * classifier_priority_map)

{
	printf ("classifier_priority_map->Priority=%d\n", LE32TOH (classifier_priority_map->Priority));
	PIBClassifierDump (&classifier_priority_map->CLASSIFIER);
	return;
}


/*====================================================================*
 *
 *   auto_connection_dump (struct auto_connection * auto_connection);
 *
 *
 *--------------------------------------------------------------------*/

void auto_connection_dump (struct auto_connection * auto_connection)

{
	unsigned count;
	printf ("auto_connection->MACTION=%d\n", auto_connection->MACTION);
	printf ("auto_connection->MOPERAND=%d\n", auto_connection->MOPERAND);
	printf ("auto_connection->NUM_CLASSIFIERS=%d\n", LE16TOH (auto_connection->NUM_CLASSIFIERS));
	for (count = 0; count < LE16TOH (auto_connection->NUM_CLASSIFIERS); count++)
	{
		PIBClassifierDump (&auto_connection->CLASSIFIER [count]);
	}
	cspec_dump (&auto_connection->cspec);
	return;
}


/*====================================================================*
 *
 *   PIBClassifiersDump (struct PIBClassifiers * classifiers);
 *
 *
 *--------------------------------------------------------------------*/

void PIBClassifiersDump (struct PIBClassifiers * classifiers)

{
	unsigned count;
	printf ("classifiers->priority_count=%d\n", LE32TOH (classifiers->priority_count));
	printf ("classifiers->autoconn_count=%d\n", LE32TOH (classifiers->autoconn_count));
	printf ("\n");
	printf ("-------- PRIORITY MAPS --------\n\n");
	for (count = 0; count < LE32TOH (classifiers->priority_count); count++)
	{
		classifier_priority_map_dump (&classifiers->classifier_priority_map [count]);
	}
	printf ("\n");
	printf ("-------- AUTO CONNECTIONS --------\n\n");
	for (count = 0; count < LE32TOH (classifiers->autoconn_count); count++)
	{
		auto_connection_dump (&classifiers->auto_connection [count]);
	}
	printf ("\n");
	return;
}


/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

