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
 *   pib.h - PIB version definitions and declarations;
 *
 *   The PIB undergoes periodic revision as new features are added;
 *   versions are distiguished from one another by the first two bytes;
 *
 *   this file declares PIB header structure for PIBs released to
 *   date; it does not define the position of all PIB information
 *   because there must be some mystery to life;
 *
 *   call function pibpeek() to display a buffer holding an unkown
 *   PIB structure;
 *
 *.  Qualcomm Atheros HomePlug AV Powerline Toolkit;
 *:  Copyright (c) 2006-2010 by Intellon Corporation; ALL RIGHTS RESERVED;
 *;  For demonstration and evaluation only; Not for production use.
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef PIB_HEADER
#define PIB_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6 /* normally defined in ethernet.h or if_ether.h */
#endif

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"
#include "../tools/memory.h"
#include "../key/HPAVKey.h"

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define PIB_NAME_LEN 32
#define PIB_HFID_LEN 64
#define PIB_TEXT_LEN 256
#define PIB_KEY_LEN 16

#define PIB_SILENCE     (1 << 0)
#define PIB_VERBOSE     (1 << 1)
#define PIB_MANIFEST    (1 << 2)
#define PIB_MAC         (1 << 3)
#define PIB_MACINC      (1 << 4)
#define PIB_NMK         (1 << 5)
#define PIB_DAK         (1 << 6)
#define PIB_MFGSTRING   (1 << 7)
#define PIB_USER        (1 << 8)
#define PIB_NETWORK     (1 << 9)
#define PIB_CCO_MODE    (1 << 10)
#define PIB_NID         (1 << 11)
#define PIB_CHECKED     (1 << 12)

#define INT_PRESCALER_OFFSET 0x0A10
#define INT_PRESCALER_LENGTH (1155 * sizeof (uint32_t))
#define AMP_PRESCALER_OFFSET 0x0A30
#define AMP_PRESCALER_LENGTH ((2880 * 5) / sizeof (uint32_t))
#define PLC_PRESCALER_OFFSET 0x0F2B
#define PLC_PRESCALER_LENGTH (1345 / sizeof (uint32_t))
#define QCA_PRESCALER_OFFSET 0x12E8
#define QCA_PRESCALER_LENGTH 1345

#define INDEX_TO_FREQ(index) ((float)((index)+74)/40.96)
#define FREQ_TO_INDEX(freq)  ((unsigned)(40.96*(freq))-74)

/*====================================================================*
 *   standard PIB message formats;
 *--------------------------------------------------------------------*/

#define PIB_BADVERSION "%s found bad PIB version in %s", __func__
#define PIB_BADCONTENT "%s found wrong PIB content in %s", __func__
#define PIB_BADLENGTH "%s found wrong PIB image length in %s", __func__
#define PIB_BADCHECKSUM "%s found wrong PIB image checksum in %s", __func__
#define PIB_BADNID "%s found wrong Preferred NID in %s", __func__

/*====================================================================*
 *   Basic PIB header;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed pib_header

{
	uint8_t FWVERSION;
	uint8_t PIBVERSION;
	uint16_t RESERVED1;
	uint16_t PIBLENGTH;
	uint16_t RESERVED2;
	uint32_t CHECKSUM;
}

pib_header;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   As of PIB 1.4 a generic structure evolved; this is the start;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed simple_pib

{
	uint16_t PIBVERSION;
	uint16_t RESERVED1;
	uint16_t PIBLENGTH;
	uint16_t RESERVED2;
	uint32_t CHECKSUM;
	uint8_t MAC [ETHER_ADDR_LEN];
	uint8_t DAK [HPAVKEY_DAK_LEN];
	uint16_t RESERVED3;
	uint8_t MFG [PIB_HFID_LEN];
	uint8_t NMK [HPAVKEY_NMK_LEN];
	uint8_t USR [PIB_HFID_LEN];
	uint8_t NET [PIB_HFID_LEN];
	uint8_t CCoSelection;
	uint8_t CexistModeSelect;
	uint8_t PLFreqSelect;
	uint8_t RESERVED4;
	uint8_t PreferredNID [HPAVKEY_NID_LEN];
	uint8_t AutoFWUpgradeable;
	uint8_t MDUConfiguration;
	uint8_t MDURole;
	uint8_t RESERVED5 [10];
	uint8_t StaticNetworkConfiguration [128];
	uint8_t InterfaceConfiguration [64];
}

simple_pib;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB1

{
	uint8_t FWVersion;
	uint8_t PIBVersion;
	uint16_t Reserved1;
	uint16_t PIBLength;
}

PIB1;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v1.2; this is deprecated;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB1_2

{
	uint8_t FWVersion;
	uint8_t PIBVersion;
	uint16_t Reserved1;
	uint8_t DAK [HPAVKEY_DAK_LEN];
	uint8_t NMK [HPAVKEY_NMK_LEN];
	uint8_t MAC [ETHER_ADDR_LEN];
	uint32_t FLG;
}

PIB1_2;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB Structure as of v1.3; this is deprecated;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB1_3

{
	uint8_t FWVersion;
	uint8_t PIBVersion;
	uint16_t Reserved1;
	uint8_t MAC [ETHER_ADDR_LEN];
	uint8_t DAK [HPAVKEY_DAK_LEN];
	uint16_t Reserved2;
	uint8_t MFG [PIB_HFID_LEN];
	uint8_t NMK [HPAVKEY_NMK_LEN];
	uint8_t USR [PIB_HFID_LEN];
	uint8_t NET [PIB_HFID_LEN];
}

PIB1_3;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB sub-structure introduced as of v1.4
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed VersionHeader

{
	uint8_t FWVersion;
	uint8_t PIBVersion;
	uint16_t Reserved1;
	uint16_t PIBLength;
	uint16_t Reserved2;
	uint32_t Checksum;
}

VersionHeader;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB Structure as of v1.4
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB1_4

{
	struct VersionHeader VersionHeader;
	struct __packed
	{
		uint8_t MAC [ETHER_ADDR_LEN];
		uint8_t DAK [HPAVKEY_DAK_LEN];
		uint16_t Reserved1;
		uint8_t MFG [PIB_HFID_LEN];
		uint8_t NMK [HPAVKEY_NMK_LEN];
		uint8_t USR [PIB_HFID_LEN];
		uint8_t NET [PIB_HFID_LEN];
		uint8_t CCoSelection;
		uint8_t CoexistModeSelection;
		uint8_t FreqSelection;
		uint8_t Reserved2;
	}
	LocalDeviceConfig;
}

PIB1_4;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB sub-structures introduced as of v1.5;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed LocalDeviceConfig

{
	uint8_t MAC [ETHER_ADDR_LEN];
	uint8_t DAK [HPAVKEY_DAK_LEN];
	uint16_t Reserved1;
	uint8_t MFG [PIB_HFID_LEN];
	uint8_t NMK [HPAVKEY_NMK_LEN];
	uint8_t USR [PIB_HFID_LEN];
	uint8_t NET [PIB_HFID_LEN];
	uint8_t CCoSelection;
	uint8_t CoexistModeSelect;
	uint8_t PLFreqSelection;
	uint8_t Reserved2;
	uint8_t PreferredNID [HPAVKEY_NID_LEN];
	uint8_t AutoFWUpgradeable;
	uint8_t MDUConfiguration;
	uint8_t MDURole;
	uint8_t Reserved3 [10];
}

LocalDeviceConfig;
typedef struct __packed StaticNetworkConfig

{
	uint8_t Reserved [128];
}

StaticNetworkConfig;
typedef struct __packed InterfaceConfig

{
	uint8_t Reserved [96];
}

InterfaceConfig;
typedef struct __packed IGMPConfig

{
	uint8_t Reserved [32];
}

IGMPConfig;
typedef struct __packed QoSParameters

{
	uint8_t UnicastPriority;
	uint8_t McastPriority;
	uint8_t IGMPPriority;
	uint8_t AVStreamPriority;
	uint32_t PriorityTTL [4];
	uint8_t EnableVLANOver;
	uint8_t EnableTOSOver;
	uint16_t Reserved1;
	uint32_t VLANPrioTOSPrecMatrix;
	uint8_t Reserved2 [2020];
}

QoSParameters;
typedef struct __packed ToneNotchParameters

{
	uint8_t Reserved [5120];
}

ToneNotchParameters;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v1.5
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB1_5

{
	struct VersionHeader VersionHeader;
	struct LocalDeviceConfig LocalDeviceConfig;
	struct StaticNetworkConfig StaticNetworkConfig;
	struct InterfaceConfig InterfaceConfig;
	struct IGMPConfig IGMPConfig;
	struct QoSParameters QoSParameters;
	struct ToneNotchParameters ToneNotchParameters;
}

PIB1_5;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB sub-structures introduced as of v2.0
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed FeatureConfiguration

{
	uint8_t Reserved [128];
}

FeatureConfiguration;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v2.0
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB2_0

{
	struct VersionHeader VersionHeader;
	struct LocalDeviceConfig LocalDeviceConfig;
	struct StaticNetworkConfig StaticNetworkConfig;
	struct InterfaceConfig InterfaceConfig;
	struct IGMPConfig IGMPConfig;
	struct QoSParameters QoSParameters;
	struct ToneNotchParameters ToneNotchParameters;
	struct FeatureConfiguration FeatureConfiguration;
}

PIB2_0;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB sub-structures introduced as of v3.0;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed V3_0Configuration

{
	uint32_t AVLNMembership;
	uint32_t SimpleConnectTimeout;
	uint8_t EnableLEDThroughputIndicate;
	uint8_t MidLEDThroughputThreshold;
	uint8_t HighLEDThroughputThreshold;
	uint8_t Reserved1;
	uint32_t EnableUnicastQueriesToMembers;
	uint32_t DisableExpireGroupMulticastInterval;
	uint32_t DisableLEDTestLights;
	uint8_t GPIOMap [12];
	uint8_t Reserved [8];
}

V3_0Configuration;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v3.0;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB3_0

{
	struct VersionHeader VersionHeader;
	struct LocalDeviceConfig LocalDeviceConfig;
	struct StaticNetworkConfig StaticNetworkConfig;
	struct InterfaceConfig InterfaceConfig;
	struct IGMPConfig IGMPConfig;
	struct QoSParameters QoSParameters;
	struct ToneNotchParameters ToneNotchParameters;
	struct FeatureConfiguration FeatureConfiguration;
	struct V3_0Configuration V3_0Configuration;
}

PIB3_0;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB sub-structures introduced as of v3.1;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed V3_1Configuration

{
	uint8_t Reserved [128];
}

V3_1Configuration;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v3.1;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB3_1

{
	struct VersionHeader VersionHeader;
	struct LocalDeviceConfig LocalDeviceConfig;
	struct StaticNetworkConfig StaticNetworkConfig;
	struct InterfaceConfig InterfaceConfig;
	struct IGMPConfig IGMPConfig;
	struct QoSParameters QoSParameters;
	struct ToneNotchParameters ToneNotchParameters;
	struct FeatureConfiguration FeatureConfiguration;
	struct V3_0Configuration V3_0Configuration;
	struct V3_1Configuration V3_1Configuration;
}

PIB3_1;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v3.2
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB3_2

{
	struct VersionHeader VersionHeader;
	struct LocalDeviceConfig LocalDeviceConfig;
	struct StaticNetworkConfig StaticNetworkConfig;
	struct InterfaceConfig InterfaceConfig;
	struct IGMPConfig IGMPConfig;
	struct QoSParameters QoSParameters;
	struct ToneNotchParameters ToneNotchParameters;
	struct FeatureConfiguration FeatureConfiguration;
	struct V3_0Configuration V3_0Configuration;
	struct V3_1Configuration V3_1Configuration;
}

PIB3_2;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB sub-structure introduced as of v3.3;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed V3_3Configuration

{
	uint8_t Reserved [64];
}

V3_3Configuration;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v3.3
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB3_3

{
	struct VersionHeader VersionHeader;
	struct LocalDeviceConfig LocalDeviceConfig;
	struct StaticNetworkConfig StaticNetworkConfig;
	struct InterfaceConfig InterfaceConfig;
	struct IGMPConfig IGMPConfig;
	struct QoSParameters QoSParameters;
	struct ToneNotchParameters ToneNotchParameters;
	struct FeatureConfiguration FeatureConfiguration;
	struct V3_0Configuration V3_0Configuration;
	struct V3_1Configuration V3_1Configuration;
	struct V3_3Configuration V3_3Configuration;
}

PIB3_3;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v3.4
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB3_4

{
	struct VersionHeader VersionHeader;
	struct LocalDeviceConfig LocalDeviceConfig;
	struct StaticNetworkConfig StaticNetworkConfig;
	struct InterfaceConfig InterfaceConfig;
	struct IGMPConfig IGMPConfig;
	struct QoSParameters QoSParameters;
	struct ToneNotchParameters ToneNotchParameters;
	struct FeatureConfiguration FeatureConfiguration;
	struct V3_0Configuration V3_0Configuration;
	struct V3_1Configuration V3_1Configuration;
	struct V3_3Configuration V3_3Configuration;
}

PIB3_4;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB sub-structure introduced as of v3.5
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed FeatureGroupEnablement

{
	uint8_t Reserved [16];
}

FeatureGroupEnablement;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v3.5
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB3_5

{
	struct VersionHeader VersionHeader;
	struct LocalDeviceConfig LocalDeviceConfig;
	struct StaticNetworkConfig StaticNetworkConfig;
	struct InterfaceConfig InterfaceConfig;
	struct IGMPConfig IGMPConfig;
	struct QoSParameters QoSParameters;
	struct ToneNotchParameters ToneNotchParameters;
	struct FeatureConfiguration FeatureConfiguration;
	struct V3_0Configuration V3_0Configuration;
	struct V3_1Configuration V3_1Configuration;
	struct V3_3Configuration V3_3Configuration;
	struct FeatureGroupEnablement FeatureGroupEnablement;
}

PIB3_5;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   PIB structure as of v3.6
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed PIB3_6

{
	struct VersionHeader VersionHeader;
	struct LocalDeviceConfig LocalDeviceConfig;
	struct StaticNetworkConfig StaticNetworkConfig;
	struct InterfaceConfig InterfaceConfig;
	struct IGMPConfig IGMPConfig;
	struct QoSParameters QoSParameters;
	struct ToneNotchParameters ToneNotchParameters;
	struct FeatureConfiguration FeatureConfiguration;
	struct V3_0Configuration V3_0Configuration;
	struct V3_1Configuration V3_1Configuration;
	struct V3_3Configuration V3_3Configuration;
	struct FeatureGroupEnablement FeatureGroupEnablement;
}

PIB3_6;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   functions;
 *--------------------------------------------------------------------*/

signed pibseek (signed fd, char const * filename, flag_t flags);
signed pibfile (struct _file_ const * pib);
signed pibfile1 (struct _file_ const * pib);
signed pibfile2 (struct _file_ const * pib);
signed piblock (struct _file_ const * pib);
signed pibpeek1 (void const * memory);
signed pibpeek2 (void const * memory);
uint16_t pibscalers (struct _file_ * pib);
uint16_t psread (uint16_t values [], uint16_t limit, FILE * fp);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

