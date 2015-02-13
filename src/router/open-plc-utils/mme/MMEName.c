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
 *   char const * MMEName (uint16_t MMTYPE);
 *
 *   mme.h
 *
 *   Return HomePlug or Atheros Management Message name for a given
 *   MMTYPE; this function is not needed but it could be useful when
 *   developing and debugging applications;
 *
 *   If you add or remove items in this list then update constant
 *   MMTYPES to reflect the number of list members;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef MMENAME_SOURCE
#define MMENAME_SOURCE

#include "../mme/mme.h"

static const struct mme_name

{
	uint16_t type;
	char const * name;
}

mme_names [] =

{


	{
		CC_CCO_APPOINT,
		"CC_CCO_APPOINT"
	},
	{
		CC_BACKUP_APPOINT,
		"CC_BACKUP_APPOINT"
	},
	{
		CC_LINK_INFO,
		"CC_LINK_INFO"
	},
	{
		CC_HANDOVER,
		"CC_HANDOVER"
	},
	{
		CC_HANDOVER_INFO,
		"CC_HANDOVER_INFO"
	},
	{
		CC_DISCOVER_LIST,
		"CC_DISCOVER_LIST"
	},
	{
		CC_LINK_NEW,
		"CC_LINK_NEW"
	},
	{
		CC_LINK_MOD,
		"CC_LINK_MOD"
	},
	{
		CC_LINK_SQZ,
		"CC_LINK_SQZ"
	},
	{
		CC_LINK_REL,
		"CC_LINK_REL"
	},
	{
		CC_DETECT_REPORT,
		"CC_DETECT_REPORT"
	},
	{
		CC_WHO_RU,
		"CC_WHO_RU"
	},
	{
		CC_ASSOC,
		"CC_ASSOC"
	},
	{
		CC_LEAVE,
		"CC_LEAVE"
	},
	{
		CC_SET_TEI_MAP,
		"CC_SET_TEI_MAP"
	},
	{
		CC_RELAY,
		"CC_RELAY"
	},
	{
		CC_BEACON_RELIABILITY,
		"CC_BEACON_RELIABILITY"
	},
	{
		CC_ALLOC_MOVE,
		"CC_ALLOC_MOVE"
	},
	{
		CC_ACCESS_NEW,
		"CC_ACCESS_NEW"
	},
	{
		CC_ACCESS_REL,
		"CC_ACCESS_REL"
	},
	{
		CC_DCPPC,
		"CC_DCPPC"
	},
	{
		CC_HP1_DET,
		"CC_HP1_DET"
	},
	{
		CC_BLE_UPDATE,
		"CC_BLE_UPDATE"
	},
	{
		CP_PROXY_APPOINT,
		"CP_PROXY_APPOINT"
	},
	{
		PH_PROXY_APPOINT,
		"PH_PROXY_APPOINT"
	},
	{
		CP_PROXY_WAKE,
		"CP_PROXY_WAKE"
	},
	{
		NN_INL,
		"NN_INL"
	},
	{
		NN_NEW_NET,
		"NN_NEW_NET"
	},
	{
		NN_ADD_ALLOC,
		"NN_ADD_ALLOC"
	},
	{
		NN_REL_ALLOC,
		"NN_REL_ALLOC"
	},
	{
		NN_REL_NET,
		"NN_REL_NET"
	},
	{
		CM_ASSOCIATED_STA,
		"CM_ASSOCIATED_STA"
	},
	{
		CM_ENCRYPTED_PAYLOAD,
		"CM_ENCRYPTED_PAYLOAD"
	},
	{
		CM_SET_KEY,
		"CM_SET_KEY"
	},
	{
		CM_GET_KEY,
		"CM_GET_KEY"
	},
	{
		CM_SC_JOIN,
		"CM_SC_JOIN"
	},
	{
		CM_CHAN_EST,
		"CM_CHAN_EST"
	},
	{
		CM_TM_UPDATE,
		"CM_TM_UPDATE"
	},
	{
		CM_AMP_MAP,
		"CM_AMP_MAP"
	},
	{
		CM_BRG_INFO,
		"CM_BRG_INFO"
	},
	{
		CM_CONN_NEW,
		"CM_CONN_NEW"
	},
	{
		CM_CONN_REL,
		"CM_CONN_REL"
	},
	{
		CM_CONN_MOD,
		"CM_CONN_MOD"
	},
	{
		CM_CONN_INFO,
		"CM_CONN_INFO"
	},
	{
		CM_STA_CAP,
		"CM_STA_CAP"
	},
	{
		CM_NW_INFO,
		"CM_NW_INFO"
	},
	{
		CM_GET_BEACON,
		"CM_GET_BEACON"
	},
	{
		CM_HFID,
		"CM_HFID"
	},
	{
		CM_MME_ERROR,
		"CM_MME_ERROR"
	},
	{
		CM_NW_STATS,
		"CM_NW_STATS"
	},
	{
		CM_SLAC_PARAM,
		"CM_SLAC_PARAM"
	},
	{
		CM_START_ATTEN_CHAR,
		"CM_START_ATTEN_CHAR"
	},
	{
		CM_ATTEN_CHAR,
		"CM_ATTEN_CHAR"
	},
	{
		CM_PKCS_CERT,
		"CM_PKCS_CERT"
	},
	{
		CM_MNBC_SOUND,
		"CM_MNBC_SOUND"
	},
	{
		CM_VALIDATE,
		"CM_VALIDATE"
	},
	{
		CM_SLAC_MATCH,
		"CM_SLAC_MATCH"
	},
	{
		CM_SLAC_USER_DATA,
		"CM_SLAC_USER_DATA"
	},
	{
		CM_ATTEN_PROFILE,
		"CM_ATTEN_PROFILE"
	},
	{
		MS_PB_ENC,
		"MS_PB_ENC"
	},
	{
		VS_SW_VER,
		"VS_SW_VER"
	},
	{
		VS_WR_MEM,
		"VS_WR_MEM"
	},
	{
		VS_RD_MEM,
		"VS_RD_MEM"
	},
	{
		VS_ST_MAC,
		"VS_ST_MAC"
	},
	{
		VS_GET_NVM,
		"VS_GET_NVM"
	},
	{
		0xA028,
		"RESERVED"
	},
	{
		0xA02C,
		"RESERVED"
	},
	{
		VS_RS_DEV,
		"VS_RS_DEV"
	},
	{
		VS_WR_MOD,
		"VS_WR_MOD"
	},
	{
		VS_RD_MOD,
		"VS_RD_MOD"
	},
	{
		VS_MOD_NVM,
		"VS_MOD_NVM"
	},
	{
		VS_WD_RPT,
		"VS_WD_RPT"
	},
	{
		VS_LNK_STATS,
		"VS_LNK_STATS"
	},
	{
		VS_SNIFFER,
		"VS_SNIFFER"
	},
	{
		VS_NW_INFO,
		"VS_NW_INFO"
	},
	{
		0xA03C,
		"RESERVED"
	},
	{
		VS_CP_RPT,
		"VS_CP_RPT"
	},
	{
		VS_ARPC,
		"VS_ARPC"
	},
	{
		VS_SET_KEY,
		"VS_SET_KEY"
	},
	{
		VS_MFG_STRING,
		"VS_MFG_STRING"
	},
	{
		VS_RD_CBLOCK,
		"VS_RD_CBLOCK"
	},
	{
		VS_SET_SDRAM,
		"VS_SET_SDRAM"
	},
	{
		VS_HOST_ACTION,
		"VS_HOST_ACTION"
	},
	{
		VS_OP_ATTRIBUTES,
		"VS_OP_ATTRIBUTES"
	},
	{
		VS_ENET_SETTINGS,
		"VS_ENET_SETTINGS"
	},
	{
		VS_TONE_MAP_CHAR,
		"VS_TONE_MAP_CHAR"
	},
	{
		VS_NW_INFO_STATS,
		"VS_NW_INFO_STATS"
	},
	{
		VS_SLAVE_MEM,
		"VS_SLAVE_MEM"
	},
	{
		VS_FAC_DEFAULTS,
		"VS_FAC_DEFAULTS"
	},
	{
		VS_CLASSIFICATION,
		"VS_CLASSIFICATION"
	},
	{
		VS_RX_TONE_MAP_CHAR,
		"VS_RX_TONE_MAP_CHAR"
	},
	{
		VS_SET_LED_BEHAVIOR,
		"VS_SET_LED_BEHAVIOR"
	},
	{
		VS_WRITE_AND_EXECUTE_APPLET,
		"VS_WRITE_AND_EXECUTE_APPLET"
	},
	{
		VS_MDIO_COMMAND,
		"VS_MDIO_COMMAND"
	},
	{
		VS_SLAVE_REG,
		"VS_SLAVE_REG"
	},
	{
		VS_BANDWIDTH_LIMITING,
		"VS_BANDWIDTH_LIMITING"
	},
	{
		VS_SNID_OPERATION,
		"VS_SNID_OPERATION"
	},
	{
		VS_NN_MITIGATE,
		"VS_NN_MITIGATE"
	},
	{
		VS_MODULE_OPERATION,
		"VS_MODULE_OPERATION"
	},
	{
		VS_DIAG_NETWORK_PROBE,
		"VS_DIAG_NETWORK_PROBE"
	},
	{
		VS_PL_LINK_STATUS,
		"VS_PL_LINK_STATUS"
	},
	{
		VS_GPIO_STATE_CHANGE,
		"VS_GPIO_STATE_CHANGE"
	},
	{
		VS_CONN_ADD,
		"VS_CONN_ADD"
	},
	{
		VS_CONN_MOD,
		"VS_CONN_MOD"
	},
	{
		VS_CONN_REL,
		"VS_CONN_REL"
	},
	{
		VS_CONN_INFO,
		"VS_CONN_INFO"
	},
	{
		VS_MULTIPORT_LNK_STA,
		"VS_MULTIPORT_LNK_STA"
	},
	{
		VS_EM_ID_TABLE,
		"VS_EM_ID_TABLE"
	},
	{
		VS_STANDBY,
		"VS_STANDBY"
	},
	{
		VS_SLEEPSCHEDULE,
		"VS_SLEEPSCHEDULE"
	},
	{
		VS_SLEEPSCHEDULE_NOTIFICATION,
		"VS_SLEEPSCHEDULE_NOTIFICATION"
	},
	{
		0xA0EC,
		"RESERVED"
	},
	{
		VS_MICROCONTROLLER_DIAG,
		"VS_MICROCONTROLLER_DIAG"
	},
	{
		VS_GET_PROPERTY,
		"VS_GET_PROPERTY"
	},
	{
		0xA0FC,
		"RESERVED"
	},
	{
		VS_SET_PROPERTY,
		"VS_SET_PROPERTY"
	},
	{
		VS_PHYSWITCH_MDIO,
		"VS_PHYSWITCH_MDIO"
	},
	{
		VS_SELFTEST_ONETIME_CONFIG,
		"VS_SELFTEST_ONETIME_CONFIG"
	},
	{
		VS_SELFTEST_RESULTS,
		"VS_SELFTEST_RESULTS"
	},
	{
		VS_MDU_TRAFFIC_STATS,
		"VS_MDU_TRAFFIC_STATS"
	},
	{
		VS_FORWARD_CONFIG,
		"VS_FORWARD_CONFIG"
	},
	{
		0xA11C,
		"RESERVED"
	},
	{
		VS_HYBRID_INFO,
		"VS_HYBRID_INFO"
	}

};

char const * MMEName (uint16_t MMTYPE)

{
	size_t lower = 0;
	size_t upper = SIZEOF (mme_names);
	MMTYPE &= MMTYPE_MASK;
	while (lower < upper)
	{
		size_t index = (lower + upper) >> 1;
		signed order = MMTYPE - mme_names [index].type;
		if (order < 0)
		{
			upper = index - 0;
			continue;
		}
		if (order > 0)
		{
			lower = index + 1;
			continue;
		}
		return (mme_names [index].name);
	}
	return ("UNKNOWN_MESSAGE_TYPE");
}


/*====================================================================*
 *   print a multi-column list of MME codes and names on stdout;
 *--------------------------------------------------------------------*/

#if 0
#include <stdio.h>

#define COLS 4
#define WIDTH 20

int main (int argc, char const * argv [])

{
	unsigned cols = COLS;
	unsigned rows = ((SIZEOF (mme_names) + (COLS - 1)) / cols);
	unsigned row = 0;
	unsigned mme = 0;
	for (row = 0; row < rows; row++)
	{
		for (mme = row; mme < SIZEOF (mme_names); mme += rows)
		{
			printf ("%04X %-*.*s ", mme_names [mme].type, WIDTH, WIDTH, mme_names [mme].name);
		}
		printf ("\n");
	}
	return (0);
}


#endif

/*====================================================================*
 *   print a multi-column list of MME codes and names on stdout;
 *--------------------------------------------------------------------*/

#if 0
#include <stdio.h>

int main (int argc, char const * argv [])

{
	unsigned mme = 0;
	for (mme = 0; mme < SIZEOF (mme_names); mme++)
	{
		printf ("{ %s, \"%s\" },",  mme_names [mme].name, mme_names [mme].name);
//		printf ("0x%04X;%s;yes;yes;yes\n", mme_names [mme].type, mme_names [mme].name);
	}
	return (0);
}


#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

