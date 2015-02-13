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
 *   signed pibpeek1 (void const * memory);
 *
 *   pib.h
 *
 *   print PIB identity information on stdout;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef PIBPEEK1_SOURCE
#define PIBPEEK1_SOURCE

#include <stdio.h>
#include <memory.h>

#include "../tools/memory.h"
#include "../tools/number.h"
#include "../key/HPAVKey.h"
#include "../key/keys.h"
#include "../pib/pib.h"

static char const * CCoMode [] =

{
	"Auto",
	"Never",
	"Always",
	"User",
	"Covert",
	"Unknown"
};

static char const * MDURole [] =

{
	"Slave",
	"Master"
};

signed pibpeek1 (void const * memory)

{
	extern const struct key keys [KEYS];
	extern char const * CCoMode [];
	extern char const * MDURole [];
	struct PIB1 * PIB = (struct PIB1 *)(memory);
	char buffer [HPAVKEY_SHA_LEN * 3];
	size_t key;
	if (PIB->FWVersion == 1)
	{
		if (PIB->PIBVersion == 2)
		{
			struct PIB1_2 * PIB = (struct PIB1_2 *)(memory);
			printf ("\tPIB %d-%d\n", PIB->FWVersion, PIB->PIBVersion);
			printf ("\tMAC %s\n", hexstring (buffer, sizeof (buffer), PIB->MAC, sizeof (PIB->MAC)));
			printf ("\tDAK %s", hexstring (buffer, sizeof (buffer), PIB->DAK, sizeof (PIB->DAK)));
			for (key = 0; key < KEYS; key++)
			{
				if (!memcmp (keys [key].DAK, PIB->DAK, HPAVKEY_DAK_LEN))
				{
					printf (" (%s)", keys [key].phrase);
					break;
				}
			}
			printf ("\n");
			printf ("\tNMK %s", hexstring (buffer, sizeof (buffer), PIB->NMK, sizeof (PIB->NMK)));
			for (key = 0; key < KEYS; key++)
			{
				if (!memcmp (keys [key].NMK, PIB->NMK, HPAVKEY_NMK_LEN))
				{
					printf (" (%s)", keys [key].phrase);
					break;
				}
			}
			printf ("\n");
			printf ("\tFLG %s\n", hexstring (buffer, sizeof (buffer), (uint8_t *)(&PIB->FLG), sizeof (PIB->FLG)));
			return (0);
		}
		else if (PIB->PIBVersion == 3)
		{
			struct PIB1_3 * PIB = (struct PIB1_3 *)(memory);
			printf ("\tPIB %d-%d\n", PIB->FWVersion, PIB->PIBVersion);
			printf ("\tMAC %s\n", hexstring (buffer, sizeof (buffer), PIB->MAC, sizeof (PIB->MAC)));
			printf ("\tDAK %s", hexstring (buffer, sizeof (buffer), PIB->DAK, sizeof (PIB->DAK)));
			for (key = 0; key < KEYS; key++)
			{
				if (!memcmp (keys [key].DAK, PIB->DAK, HPAVKEY_DAK_LEN))
				{
					printf (" (%s)", keys [key].phrase);
					break;
				}
			}
			printf ("\n");
			printf ("\tNMK %s", hexstring (buffer, sizeof (buffer), PIB->NMK, sizeof (PIB->NMK)));
			for (key = 0; key < KEYS; key++)
			{
				if (!memcmp (keys [key].NMK, PIB->NMK, HPAVKEY_NMK_LEN))
				{
					printf (" (%s)", keys [key].phrase);
					break;
				}
			}
			printf ("\n");
			printf ("\tNET \"%s\"\n", PIB->NET);
			printf ("\tMFG \"%s\"\n", PIB->MFG);
			printf ("\tUSR \"%s\"\n", PIB->USR);
			return (0);
		}
		else if (PIB->PIBVersion >= 4)
		{
			struct PIB1_4 * PIB = (struct PIB1_4 *)(memory);
			printf ("\tPIB %d-%d %d bytes\n", PIB->VersionHeader.FWVersion, PIB->VersionHeader.PIBVersion, LE16TOH (PIB->VersionHeader.PIBLength));
			printf ("\tMAC %s\n", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.MAC, sizeof (PIB->LocalDeviceConfig.MAC)));
			printf ("\tDAK %s", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.DAK, sizeof (PIB->LocalDeviceConfig.DAK)));
			for (key = 0; key < KEYS; key++)
			{
				if (!memcmp (keys [key].DAK, PIB->LocalDeviceConfig.DAK, HPAVKEY_DAK_LEN))
				{
					printf (" (%s)", keys [key].phrase);
					break;
				}
			}
			printf ("\n");
			printf ("\tNMK %s", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.NMK, sizeof (PIB->LocalDeviceConfig.NMK)));
			for (key = 0; key < KEYS; key++)
			{
				if (!memcmp (keys [key].NMK, PIB->LocalDeviceConfig.NMK, HPAVKEY_NMK_LEN))
				{
					printf (" (%s)", keys [key].phrase);
					break;
				}
			}
			printf ("\n");
			printf ("\tNET %s\n", PIB->LocalDeviceConfig.NET);
			printf ("\tMFG %s\n", PIB->LocalDeviceConfig.MFG);
			printf ("\tUSR %s\n", PIB->LocalDeviceConfig.USR);
			return (0);
		}
	}
	else if (PIB->FWVersion == 2)
	{
		struct PIB2_0 * PIB = (struct PIB2_0 *)(memory);
		printf ("\tPIB %d-%d %d bytes\n", PIB->VersionHeader.FWVersion, PIB->VersionHeader.PIBVersion, LE16TOH (PIB->VersionHeader.PIBLength));
		printf ("\tMAC %s\n", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.MAC, sizeof (PIB->LocalDeviceConfig.MAC)));
		printf ("\tDAK %s", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.DAK, sizeof (PIB->LocalDeviceConfig.DAK)));
		for (key = 0; key < KEYS; key++)
		{
			if (!memcmp (keys [key].DAK, PIB->LocalDeviceConfig.DAK, HPAVKEY_DAK_LEN))
			{
				printf (" (%s)", keys [key].phrase);
				break;
			}
		}
		printf ("\n");
		printf ("\tNMK %s", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.NMK, sizeof (PIB->LocalDeviceConfig.NMK)));
		for (key = 0; key < KEYS; key++)
		{
			if (!memcmp (keys [key].NMK, PIB->LocalDeviceConfig.NMK, HPAVKEY_NMK_LEN))
			{
				printf (" (%s)", keys [key].phrase);
				break;
			}
		}
		printf ("\n");
		printf ("\tNID %s\n", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.PreferredNID, sizeof (PIB->LocalDeviceConfig.PreferredNID)));
		printf ("\tNET %s\n", PIB->LocalDeviceConfig.NET);
		printf ("\tMFG %s\n", PIB->LocalDeviceConfig.MFG);
		printf ("\tUSR %s\n", PIB->LocalDeviceConfig.USR);
		return (0);
	}
	else if (PIB->FWVersion < 8)
	{
		struct PIB3_0 * PIB = (struct PIB3_0 *)(memory);
		printf ("\tPIB %d-%d %d bytes\n", PIB->VersionHeader.FWVersion, PIB->VersionHeader.PIBVersion, LE16TOH (PIB->VersionHeader.PIBLength));
		printf ("\tMAC %s\n", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.MAC, sizeof (PIB->LocalDeviceConfig.MAC)));
		printf ("\tDAK %s", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.DAK, sizeof (PIB->LocalDeviceConfig.DAK)));
		for (key = 0; key < KEYS; key++)
		{
			if (!memcmp (keys [key].DAK, PIB->LocalDeviceConfig.DAK, HPAVKEY_DAK_LEN))
			{
				printf (" (%s)", keys [key].phrase);
				break;
			}
		}
		printf ("\n");
		printf ("\tNMK %s", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.NMK, sizeof (PIB->LocalDeviceConfig.NMK)));
		for (key = 0; key < KEYS; key++)
		{
			if (!memcmp (keys [key].NMK, PIB->LocalDeviceConfig.NMK, HPAVKEY_NMK_LEN))
			{
				printf (" (%s)", keys [key].phrase);
				break;
			}
		}
		printf ("\n");
		printf ("\tNID %s\n", hexstring (buffer, sizeof (buffer), PIB->LocalDeviceConfig.PreferredNID, sizeof (PIB->LocalDeviceConfig.PreferredNID)));
		printf ("\tNET %s\n", PIB->LocalDeviceConfig.NET);
		printf ("\tMFG %s\n", PIB->LocalDeviceConfig.MFG);
		printf ("\tUSR %s\n", PIB->LocalDeviceConfig.USR);
		printf ("\tCCo %s\n", CCoMode [PIB->LocalDeviceConfig.CCoSelection > SIZEOF (CCoMode)-1?SIZEOF (CCoMode)-1:PIB->LocalDeviceConfig.CCoSelection]);
		printf ("\tMDU %s\n", PIB->LocalDeviceConfig.MDUConfiguration? MDURole [PIB->LocalDeviceConfig.MDURole & 1]: "N/A");
		return (0);
	}
	printf ("\tPIB %d-%d %d (unsupported)\n", PIB->FWVersion, PIB->PIBVersion, LE16TOH (PIB->PIBLength));
	return (1);
}


#endif

